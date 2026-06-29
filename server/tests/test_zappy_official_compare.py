#!/usr/bin/env python3
import os
import re
import signal
import socket
import subprocess
import time
import unittest
from contextlib import closing
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
OFFICIAL_SERVER = Path(ROOT / "binaries" / "zappy_server_official").resolve()
MINE_SERVER = Path(ROOT / "zappy_server").resolve()
GUI_BIN = Path(ROOT / "zappy_gui").resolve()

WIDTH = 10
HEIGHT = 10
FREQ = 100
TEAMS = ("teamA", "teamB")


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
            return "<closed>"
        if chunk == b"\n":
            return data.decode("utf-8", errors="replace").rstrip("\r")
        data.extend(chunk)
    return "<timeout>" if not data else f"<timeout:{data.decode(errors='replace')}>"


def read_available(sock, idle_timeout=0.05, total_timeout=0.8, limit=1000):
    lines = []
    deadline = time.monotonic() + total_timeout
    while len(lines) < limit and time.monotonic() < deadline:
        line = read_line(sock, timeout=idle_timeout)
        if line.startswith("<timeout"):
            break
        lines.append(line)
    return lines


def send_line(sock, line):
    sock.sendall((line + "\n").encode())


def bct_summary(lines):
    coords = set()
    malformed = 0
    pattern = re.compile(r"^bct (\d+) (\d+)( \d+){7}$")
    for line in lines:
        match = pattern.fullmatch(line)
        if not match:
            malformed += 1
            continue
        coords.add((int(match.group(1)), int(match.group(2))))
    return {"count": len(lines), "coords": coords, "malformed": malformed}


def classify_lines(lines):
    prefixes = {}
    for line in lines:
        prefixes[line.split(" ", 1)[0]] = prefixes.get(line.split(" ", 1)[0], 0) + 1
    return prefixes


class ServerProcess:
    def __init__(self, binary, *, width=WIDTH, height=HEIGHT, clients=2, freq=FREQ):
        self.binary = Path(binary)
        self.width = width
        self.height = height
        self.clients = clients
        self.freq = freq
        self.port = find_free_port()
        self.proc = None

    def __enter__(self):
        args = [
            str(self.binary),
            "-p", str(self.port),
            "-x", str(self.width),
            "-y", str(self.height),
            "-n", *TEAMS,
            "-c", str(self.clients),
            "-f", str(self.freq),
        ]
        self.proc = subprocess.Popen(
            args, cwd=ROOT, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True
        )
        deadline = time.monotonic() + 1.0
        while time.monotonic() < deadline:
            if self.proc.poll() is not None:
                stderr = self.proc.stderr.read() if self.proc.stderr else ""
                raise AssertionError(
                    f"{self.binary.name} exited early: code={self.proc.returncode} stderr={stderr!r}"
                )
            try:
                sock = socket.create_connection(("127.0.0.1", self.port), timeout=0.05)
                if read_line(sock, timeout=0.2) == "WELCOME":
                    sock.close()
                    return self
                sock.close()
            except OSError:
                time.sleep(0.02)
        raise AssertionError(f"{self.binary.name} did not accept connections")

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
        return sock, read_line(sock)

    def connect_gui(self):
        sock, welcome = self.connect_raw()
        send_line(sock, "GRAPHIC")
        return sock, welcome

    def connect_ai(self, team="teamA"):
        sock, welcome = self.connect_raw()
        send_line(sock, team)
        return sock, welcome, read_line(sock), read_line(sock)


def run_on_both(callback, **server_kwargs):
    results = {}
    for label, binary in (("official", OFFICIAL_SERVER), ("mine", MINE_SERVER)):
        with ServerProcess(binary, **server_kwargs) as server:
            results[label] = callback(server)
    return results


def drain_gui_until_sgt(sock):
    lines = []
    for _ in range(300):
        line = read_line(sock, timeout=1.0)
        lines.append(line)
        if line.startswith("sgt "):
            break
    return lines


def flush_socket(sock, settle_time=0.15):
    time.sleep(settle_time)
    return read_available(sock, total_timeout=0.5)


@unittest.skipUnless(OFFICIAL_SERVER.exists() and MINE_SERVER.exists(),
                     "official and local server binaries are required")
class OfficialComparisonTests(unittest.TestCase):
    maxDiff = None

    def assert_same(self, results):
        self.assertEqual(results["mine"], results["official"], results)

    def test_valid_startup_and_first_welcome_match(self):
        def scenario(server):
            sock, welcome = server.connect_raw()
            sock.close()
            return {"welcome": welcome}

        self.assert_same(run_on_both(scenario))

    def test_argument_validation_matches_official(self):
        def run_invalid(binary):
            port = find_free_port()
            proc = subprocess.Popen(
                [str(binary), "-p", str(port), "-x", "5", "-y", "5",
                 "-n", "teamA", "teamB", "-c", "2", "-f", "100"],
                cwd=ROOT, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True
            )
            try:
                proc.wait(timeout=0.5)
                stderr = proc.stderr.read() if proc.stderr else ""
                stdout = proc.stdout.read() if proc.stdout else ""
                return {"state": "exited", "code": proc.returncode,
                        "mentions_usage": "USAGE:" in stderr + stdout}
            except subprocess.TimeoutExpired:
                proc.terminate()
                proc.wait(timeout=1)
                return {"state": "running"}
            finally:
                if proc.stdout:
                    proc.stdout.close()
                if proc.stderr:
                    proc.stderr.close()

        self.assert_same({
            "official": run_invalid(OFFICIAL_SERVER),
            "mine": run_invalid(MINE_SERVER),
        })

    def test_gui_binary_connects_to_both_servers(self):
        self.assertTrue(GUI_BIN.exists(), f"missing GUI binary: {GUI_BIN}")
        os.chmod(GUI_BIN, os.stat(GUI_BIN).st_mode | 0o111)

        def scenario(server):
            proc = subprocess.Popen(
                ["xvfb-run", "-a", str(GUI_BIN), "-h", "127.0.0.1", "-p", str(server.port)],
                cwd=ROOT, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True,
                start_new_session=True,
            )
            time.sleep(1.5)
            alive_after_connect = proc.poll() is None
            if proc.poll() is None:
                os.killpg(proc.pid, signal.SIGTERM)
                try:
                    proc.wait(timeout=1)
                except subprocess.TimeoutExpired:
                    os.killpg(proc.pid, signal.SIGKILL)
                    proc.wait(timeout=1)
            try:
                stdout, stderr = proc.communicate(timeout=1)
            except subprocess.TimeoutExpired:
                os.killpg(proc.pid, signal.SIGKILL)
                stdout, stderr = proc.communicate(timeout=1)
            return {
                "alive_after_connect": alive_after_connect,
                "has_connect_error": "Connect error" in stdout + stderr,
                "has_client_exception": "Client exception" in stdout + stderr,
            }

        self.assert_same(run_on_both(scenario))

    def test_gui_initial_state_matches_official(self):
        def scenario(server):
            gui, welcome = server.connect_gui()
            lines = drain_gui_until_sgt(gui)
            gui.close()
            bcts = [line for line in lines if line.startswith("bct ")]
            return {
                "welcome": welcome,
                "first": lines[0] if lines else None,
                "last": lines[-1] if lines else None,
                "prefix_counts": classify_lines(lines),
                "bct": bct_summary(bcts),
                "teams": sorted(line for line in lines if line.startswith("tna ")),
                "egg_count": sum(line.startswith("enw ") for line in lines),
            }

        self.assert_same(run_on_both(scenario))

    def test_gui_query_commands_match_official(self):
        def scenario(server):
            gui, welcome = server.connect_gui()
            drain_gui_until_sgt(gui)
            flush_socket(gui)
            result = {"welcome": welcome}

            send_line(gui, "msz")
            result["msz"] = read_line(gui)

            send_line(gui, "sgt")
            result["sgt"] = read_line(gui)

            send_line(gui, "tna")
            result["tna"] = sorted(read_available(gui, total_timeout=0.4))

            send_line(gui, "mct")
            mct = [read_line(gui, timeout=1.0) for _ in range(WIDTH * HEIGHT)]
            result["mct_prefixes"] = classify_lines(mct)
            result["mct_bct"] = bct_summary([line for line in mct if line.startswith("bct ")])

            send_line(gui, "bct 0 0")
            result["bct_shape"] = bool(re.fullmatch(r"bct 0 0( \d+){7}", read_line(gui)))

            for command in ("wat", "msz extra", "bct -1 0", "ppo 1", "sst 0", "sst 1001"):
                send_line(gui, command)
                result[f"error:{command}"] = read_line(gui)

            gui.close()
            return result

        self.assert_same(run_on_both(scenario))

    def test_ai_handshake_and_basic_commands_match_official(self):
        look_pattern = re.compile(
            r"^\[( ?(player|food|linemate|deraumere|sibur|mendiane|phiras|thystame)"
            r"( (player|food|linemate|deraumere|sibur|mendiane|phiras|thystame))*"
            r")?(,( ?(player|food|linemate|deraumere|sibur|mendiane|phiras|thystame)"
            r"( (player|food|linemate|deraumere|sibur|mendiane|phiras|thystame))*"
            r")?)* ?\]$"
        )

        def scenario(server):
            ai, welcome, slots, size = server.connect_ai("teamA")
            result = {"welcome": welcome, "slots": slots, "size": size}
            for command in ("Connect_nbr", "Inventory"):
                send_line(ai, command)
                result[command] = read_line(ai, timeout=2.0)
            send_line(ai, "Look")
            result["Look_format_ok"] = bool(look_pattern.fullmatch(
                read_line(ai, timeout=2.0)))
            ai.close()
            return result

        self.assert_same(run_on_both(scenario))

    def test_ai_unknown_command_matches_official(self):
        def scenario(server):
            ai, _, _, _ = server.connect_ai("teamA")
            send_line(ai, "Nope")
            response = read_line(ai)
            ai.close()
            return response

        self.assert_same(run_on_both(scenario))

    def test_ai_movement_responses_and_gui_events_match_official(self):
        def scenario(server):
            gui, _ = server.connect_gui()
            drain_gui_until_sgt(gui)
            flush_socket(gui)
            ai, _, _, _ = server.connect_ai("teamA")
            read_available(gui, total_timeout=0.2)
            result = {}
            for command in ("Right", "Left", "Forward"):
                send_line(ai, command)
                result[f"{command}:ai"] = read_line(ai, timeout=2.0)
                events = read_available(gui, total_timeout=0.4)
                result[f"{command}:event_prefixes"] = classify_lines(events)
                result[f"{command}:has_ppo"] = any(line.startswith("ppo #") for line in events)
            ai.close()
            gui.close()
            return result

        self.assert_same(run_on_both(scenario))

    def test_ai_broadcast_and_fork_match_official(self):
        def scenario(server):
            gui, _ = server.connect_gui()
            drain_gui_until_sgt(gui)
            flush_socket(gui)
            ai, _, _, _ = server.connect_ai("teamA")
            read_available(gui, total_timeout=0.2)
            result = {}

            send_line(ai, "Broadcast hello world")
            result["broadcast_ai"] = read_line(ai, timeout=2.0)
            events = read_available(gui, total_timeout=0.4)
            result["broadcast_has_pbc"] = any(line.startswith("pbc #") and line.endswith("hello world")
                                              for line in events)

            send_line(ai, "Fork")
            result["fork_ai"] = read_line(ai, timeout=2.0)
            events = read_available(gui, total_timeout=0.8)
            result["fork_prefixes"] = classify_lines(events)

            send_line(ai, "Connect_nbr")
            result["slots_after_fork"] = read_line(ai, timeout=1.0)

            ai.close()
            gui.close()
            return result

        self.assert_same(run_on_both(scenario))

    def test_ai_take_set_eject_responses_match_official(self):
        def scenario(server):
            ai, _, _, _ = server.connect_ai("teamA")
            result = {}
            for command in ("Take food", "Set food", "Eject"):
                send_line(ai, command)
                result[command] = read_line(ai, timeout=1.5)
            ai.close()
            return result

        self.assert_same(run_on_both(scenario))

    def test_team_capacity_behavior_matches_official(self):
        def scenario(server):
            first, _, slots, size = server.connect_ai("teamA")
            second, welcome = server.connect_raw()
            send_line(second, "teamA")
            response = read_line(second)
            first.close()
            second.close()
            return {"first_slots": slots, "first_size": size,
                    "second_welcome": welcome, "second_response": response}

        self.assert_same(run_on_both(scenario, clients=1))


if __name__ == "__main__":
    unittest.main(verbosity=2)
