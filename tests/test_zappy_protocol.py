#!/usr/bin/env python3
import os
import re
import socket
import subprocess
import time
import unittest
from contextlib import closing
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
DEFAULT_SERVER = ROOT / "build" / "server-tests" / "zappy_server"
SERVER_BIN = Path(os.environ.get("ZAPPY_SERVER_BIN", DEFAULT_SERVER)).resolve()
ROOT_SERVER_BIN = ROOT / "zappy_server"
GUI_APPIMAGE = ROOT / "zappy_gui.AppImage"


def find_free_port():
    with closing(socket.socket(socket.AF_INET, socket.SOCK_STREAM)) as sock:
        sock.bind(("127.0.0.1", 0))
        return sock.getsockname()[1]


def read_line(sock, timeout=1.0):
    sock.settimeout(timeout)
    data = bytearray()
    deadline = time.monotonic() + timeout
    while time.monotonic() < deadline:
        try:
            chunk = sock.recv(1)
        except socket.timeout:
            break
        if not chunk:
            raise AssertionError("socket closed while waiting for a line")
        if chunk == b"\n":
            return data.decode("utf-8", errors="replace").rstrip("\r")
        data.extend(chunk)
    raise AssertionError(f"timeout while waiting for a line; partial={data!r}")


def read_available_lines(sock, idle_timeout=0.05, total_timeout=1.0):
    lines = []
    deadline = time.monotonic() + total_timeout
    while time.monotonic() < deadline:
        try:
            lines.append(read_line(sock, timeout=idle_timeout))
        except AssertionError:
            break
    return lines


def send_line(sock, line):
    sock.sendall((line + "\n").encode())


class ZappyServer:
    def __init__(self, *, width=5, height=5, clients=4, freq=100,
                 teams=("teamA", "teamB"), binary=SERVER_BIN):
        self.width = width
        self.height = height
        self.clients = clients
        self.freq = freq
        self.teams = teams
        self.binary = Path(binary)
        self.port = find_free_port()
        self.proc = None

    def __enter__(self):
        args = [
            str(self.binary),
            "-p", str(self.port),
            "-x", str(self.width),
            "-y", str(self.height),
            "-n", *self.teams,
            "-c", str(self.clients),
            "-f", str(self.freq),
        ]
        self.proc = subprocess.Popen(
            args,
            cwd=ROOT,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True,
        )
        deadline = time.monotonic() + 1.0
        last_error = None
        while time.monotonic() < deadline:
            if self.proc.poll() is not None:
                stderr = self.proc.stderr.read() if self.proc.stderr else ""
                raise AssertionError(
                    f"server exited early with code {self.proc.returncode}: {stderr.strip()}"
                )
            try:
                with socket.create_connection(("127.0.0.1", self.port), timeout=0.05) as sock:
                    read_line(sock, timeout=0.2)
                return self
            except OSError as error:
                last_error = error
                time.sleep(0.02)
            except AssertionError:
                return self
        raise AssertionError(f"server did not accept connections: {last_error}")

    def __exit__(self, exc_type, exc, tb):
        if self.proc and self.proc.poll() is None:
            self.proc.terminate()
            try:
                self.proc.wait(timeout=1)
            except subprocess.TimeoutExpired:
                self.proc.kill()
                self.proc.wait(timeout=1)
        if self.proc:
            if self.proc.stdout:
                self.proc.stdout.close()
            if self.proc.stderr:
                self.proc.stderr.close()

    def connect_raw(self):
        sock = socket.create_connection(("127.0.0.1", self.port), timeout=1)
        self._assert_welcome(sock)
        return sock

    def connect_gui(self):
        sock = self.connect_raw()
        send_line(sock, "GRAPHIC")
        return sock

    def connect_ai(self, team="teamA"):
        sock = self.connect_raw()
        send_line(sock, team)
        slots = read_line(sock)
        size = read_line(sock)
        return sock, slots, size

    @staticmethod
    def _assert_welcome(sock):
        line = read_line(sock)
        if line != "WELCOME":
            raise AssertionError(f"expected WELCOME, got {line!r}")


def drain_gui_initial_state(sock, expected_freq=100):
    lines = []
    expected_last_line = f"sgt {expected_freq}"
    while not lines or lines[-1] != expected_last_line:
        lines.append(read_line(sock))
    return lines


class BinarySmokeTests(unittest.TestCase):
    def test_root_server_binary_stays_alive_with_valid_arguments(self):
        self.assertTrue(ROOT_SERVER_BIN.exists(), "missing ./zappy_server")
        with ZappyServer(binary=ROOT_SERVER_BIN) as server:
            self.assertIsNone(server.proc.poll())

    def test_gui_appimage_exists_and_is_executable(self):
        self.assertTrue(GUI_APPIMAGE.exists(), "missing ./zappy_gui.AppImage")
        self.assertTrue(os.access(GUI_APPIMAGE, os.X_OK), "GUI AppImage is not executable")


@unittest.skipUnless(SERVER_BIN.exists(), f"server binary not found: {SERVER_BIN}")
class HandshakeTests(unittest.TestCase):
    def test_ai_connection_returns_slots_and_world_size(self):
        with ZappyServer(width=7, height=6, clients=3) as server:
            ai, slots, size = server.connect_ai("teamA")
            self.addCleanup(ai.close)
            self.assertEqual(slots, "2")
            self.assertEqual(size, "7 6")

    def test_ai_unknown_team_is_rejected(self):
        with ZappyServer() as server:
            client = server.connect_raw()
            self.addCleanup(client.close)
            send_line(client, "unknown")
            self.assertEqual(read_line(client), "ko")

    def test_gui_initial_state_contains_map_teams_eggs_and_frequency(self):
        with ZappyServer(width=4, height=3, clients=2) as server:
            gui = server.connect_gui()
            self.addCleanup(gui.close)
            lines = drain_gui_initial_state(gui)
            self.assertEqual(lines[0], "msz 4 3")
            self.assertEqual(len([line for line in lines if line.startswith("bct ")]), 12)
            self.assertIn("tna teamA", lines)
            self.assertIn("tna teamB", lines)
            self.assertEqual(len([line for line in lines if line.startswith("enw ")]), 4)

    def test_gui_receives_ai_connection_events(self):
        with ZappyServer() as server:
            gui = server.connect_gui()
            self.addCleanup(gui.close)
            drain_gui_initial_state(gui)

            ai, _, _ = server.connect_ai("teamA")
            self.addCleanup(ai.close)
            events = read_available_lines(gui, total_timeout=1.0)
            self.assertTrue(any(re.fullmatch(r"ebo #\d+", line) for line in events), events)
            self.assertTrue(any(re.fullmatch(r"pnw #1 \d+ \d+ 1 1 teamA", line) for line in events), events)
            self.assertTrue(any(re.fullmatch(r"pin #1 \d+ \d+ 10 0 0 0 0 0 0", line) for line in events), events)


class GuiCommandTests(unittest.TestCase):
    def test_gui_basic_queries(self):
        with ZappyServer(width=3, height=2, teams=("red", "blue")) as server:
            gui = server.connect_gui()
            self.addCleanup(gui.close)
            drain_gui_initial_state(gui)

            send_line(gui, "msz")
            self.assertEqual(read_line(gui), "msz 3 2")
            send_line(gui, "bct 0 0")
            self.assertRegex(read_line(gui), r"^bct 0 0( \d+){7}$")
            send_line(gui, "mct")
            self.assertEqual(len([read_line(gui) for _ in range(6)]), 6)
            send_line(gui, "tna")
            self.assertEqual({read_line(gui), read_line(gui)}, {"tna red", "tna blue"})

    def test_gui_errors_and_frequency_update(self):
        with ZappyServer() as server:
            gui = server.connect_gui()
            self.addCleanup(gui.close)
            drain_gui_initial_state(gui)

            for command in ("wat", "", "msz extra", "bct -1 0", "ppo 1", "sst 0", "sst 1001"):
                send_line(gui, command)
                expected = "suc" if command in ("wat", "") else "sbp"
                self.assertEqual(read_line(gui), expected, command)

            send_line(gui, "sgt")
            self.assertEqual(read_line(gui), "sgt 100")
            send_line(gui, "sst 250")
            self.assertEqual(read_line(gui), "sst 250")
            send_line(gui, "sgt")
            self.assertEqual(read_line(gui), "sgt 250")

    def test_gui_player_queries_after_ai_connection(self):
        with ZappyServer() as server:
            ai, _, _ = server.connect_ai("teamA")
            self.addCleanup(ai.close)
            gui = server.connect_gui()
            self.addCleanup(gui.close)
            initial = drain_gui_initial_state(gui)

            self.assertTrue(any(line.startswith("pnw #1 ") for line in initial), initial)
            send_line(gui, "ppo #1")
            self.assertRegex(read_line(gui), r"^ppo #1 \d+ \d+ 1$")
            send_line(gui, "plv #1")
            self.assertEqual(read_line(gui), "plv #1 1")
            send_line(gui, "pin #1")
            self.assertRegex(read_line(gui), r"^pin #1 \d+ \d+ 10 0 0 0 0 0 0$")
            send_line(gui, "ppo #999")
            self.assertEqual(read_line(gui), "sbp")


class AiCommandTests(unittest.TestCase):
    def test_ai_immediate_and_scheduled_commands(self):
        with ZappyServer(freq=100) as server:
            gui = server.connect_gui()
            self.addCleanup(gui.close)
            drain_gui_initial_state(gui)
            ai, slots, size = server.connect_ai("teamA")
            self.addCleanup(ai.close)
            self.assertEqual(slots, "3")
            self.assertEqual(size, "5 5")
            read_available_lines(gui, total_timeout=0.2)

            send_line(ai, "Connect_nbr")
            self.assertEqual(read_line(ai), "3")
            send_line(ai, "Inventory")
            self.assertEqual(read_line(ai), "[food 10, linemate 0, deraumere 0, sibur 0, mendiane 0, phiras 0, thystame 0]")
            send_line(ai, "Look")
            self.assertRegex(read_line(ai), r"^\[.*\]$")

            send_line(ai, "Right")
            self.assertEqual(read_line(ai), "ok")
            self.assertTrue(any(re.fullmatch(r"ppo #1 \d+ \d+ 2", line)
                                for line in read_available_lines(gui, total_timeout=0.5)))

            send_line(ai, "Left")
            self.assertEqual(read_line(ai), "ok")
            self.assertTrue(any(re.fullmatch(r"ppo #1 \d+ \d+ 1", line)
                                for line in read_available_lines(gui, total_timeout=0.5)))

            send_line(ai, "Forward")
            self.assertEqual(read_line(ai), "ok")
            self.assertTrue(any(re.fullmatch(r"ppo #1 \d+ \d+ 1", line)
                                for line in read_available_lines(gui, total_timeout=0.5)))

    def test_ai_broadcast_notifies_gui(self):
        with ZappyServer(freq=100) as server:
            gui = server.connect_gui()
            self.addCleanup(gui.close)
            drain_gui_initial_state(gui)
            ai, _, _ = server.connect_ai("teamA")
            self.addCleanup(ai.close)
            read_available_lines(gui, total_timeout=0.2)

            send_line(ai, "Broadcast hello world")
            self.assertEqual(read_line(ai), "ok")
            self.assertIn("pbc #1 hello world", read_available_lines(gui, total_timeout=0.5))

    def test_ai_fork_notifies_gui_and_increases_slots(self):
        with ZappyServer(freq=100) as server:
            gui = server.connect_gui()
            self.addCleanup(gui.close)
            drain_gui_initial_state(gui)
            ai, _, _ = server.connect_ai("teamA")
            self.addCleanup(ai.close)
            read_available_lines(gui, total_timeout=0.2)

            send_line(ai, "Fork")
            self.assertEqual(read_line(ai, timeout=1.0), "ok")
            events = read_available_lines(gui, total_timeout=0.5)
            self.assertIn("pfk #1", events)
            self.assertTrue(any(re.fullmatch(r"enw #\d+ #1 \d+ \d+", line) for line in events), events)
            send_line(ai, "Connect_nbr")
            self.assertEqual(read_line(ai), "4")

    def test_ai_unknown_command_should_follow_protocol(self):
        with ZappyServer() as server:
            ai, _, _ = server.connect_ai("teamA")
            self.addCleanup(ai.close)
            send_line(ai, "Nope")
            self.assertEqual(read_line(ai), "ko")

    def test_ai_unsupported_actions_should_answer_ok_or_ko(self):
        with ZappyServer(freq=100) as server:
            ai, _, _ = server.connect_ai("teamA")
            self.addCleanup(ai.close)
            for command in ("Take food", "Set food", "Eject"):
                with self.subTest(command=command):
                    send_line(ai, command)
                    self.assertIn(read_line(ai, timeout=0.5), {"ok", "ko"})

    def test_ai_command_queue_limit_reports_overflow(self):
        with ZappyServer(freq=1) as server:
            ai, _, _ = server.connect_ai("teamA")
            self.addCleanup(ai.close)
            for _ in range(11):
                send_line(ai, "Inventory")
            self.assertEqual(read_line(ai, timeout=1.0), "ko")


class CapacityTests(unittest.TestCase):
    def test_team_capacity_rejects_extra_ai_clients(self):
        with ZappyServer(clients=1, teams=("solo",)) as server:
            first, slots, size = server.connect_ai("solo")
            self.addCleanup(first.close)
            self.assertEqual(slots, "0")
            self.assertEqual(size, "5 5")

            second = server.connect_raw()
            self.addCleanup(second.close)
            send_line(second, "solo")
            self.assertEqual(read_line(second), "ko")


if __name__ == "__main__":
    unittest.main(verbosity=2)
