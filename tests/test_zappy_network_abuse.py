#!/usr/bin/env python3
import os
import socket
import subprocess
import time
import unittest
from contextlib import closing
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
DEFAULT_SERVER = ROOT / "zappy_server"
SERVER_BIN = Path(os.environ.get("ZAPPY_SERVER_BIN", DEFAULT_SERVER)).resolve()

WIDTH = 10
HEIGHT = 10
FREQ = 1000


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
        except OSError:
            return "<closed>"
        if not chunk:
            return "<closed>"
        if chunk == b"\n":
            return data.decode("utf-8", errors="replace").rstrip("\r")
        data.extend(chunk)
    if data:
        return "<timeout:" + data.decode("utf-8", errors="replace") + ">"
    return "<timeout>"


def read_available(sock, idle_timeout=0.02, total_timeout=0.25, limit=1000):
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


class ZappyServer:
    def __init__(self, *, clients=8, teams=("teamA", "teamB")):
        self.clients = clients
        self.teams = teams
        self.port = find_free_port()
        self.proc = None

    def __enter__(self):
        args = [
            str(SERVER_BIN),
            "-p", str(self.port),
            "-x", str(WIDTH),
            "-y", str(HEIGHT),
            "-n", *self.teams,
            "-c", str(self.clients),
            "-f", str(FREQ),
        ]
        self.proc = subprocess.Popen(
            args, cwd=ROOT, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True
        )
        deadline = time.monotonic() + 2.0
        while time.monotonic() < deadline:
            if self.proc.poll() is not None:
                stderr = self.proc.stderr.read() if self.proc.stderr else ""
                raise AssertionError(
                    f"server exited early with code {self.proc.returncode}: {stderr.strip()}"
                )
            try:
                sock = socket.create_connection(("127.0.0.1", self.port), timeout=0.05)
                if read_line(sock, timeout=0.2) == "WELCOME":
                    sock.close()
                    return self
                sock.close()
            except OSError:
                time.sleep(0.02)
        raise AssertionError("server did not accept connections")

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

    def assert_alive(self):
        if self.proc.poll() is not None:
            stderr = self.proc.stderr.read() if self.proc.stderr else ""
            raise AssertionError(
                f"server crashed with code {self.proc.returncode}: {stderr.strip()}"
            )

    def connect_raw(self):
        sock = socket.create_connection(("127.0.0.1", self.port), timeout=1)
        self.assertEqualWelcome(sock)
        return sock

    def connect_ai(self, team="teamA"):
        sock = self.connect_raw()
        send_line(sock, team)
        slots = read_line(sock)
        size = read_line(sock)
        return sock, slots, size

    @staticmethod
    def assertEqualWelcome(sock):
        welcome = read_line(sock)
        if welcome != "WELCOME":
            raise AssertionError(f"expected WELCOME, got {welcome!r}")


@unittest.skipUnless(SERVER_BIN.exists(), f"server binary not found: {SERVER_BIN}")
class NetworkAbuseTests(unittest.TestCase):
    def test_fragmented_team_name_is_reassembled(self):
        with ZappyServer() as server:
            client = server.connect_raw()
            self.addCleanup(client.close)
            client.sendall(b"tea")
            self.assertEqual(read_line(client, timeout=0.2), "<timeout>")
            client.sendall(b"mA\n")
            self.assertRegex(read_line(client, timeout=1.0), r"^\d+$")
            self.assertEqual(read_line(client, timeout=1.0), f"{WIDTH} {HEIGHT}")
            server.assert_alive()

    def test_fragmented_ai_command_is_reassembled(self):
        with ZappyServer() as server:
            ai, _, _ = server.connect_ai("teamA")
            self.addCleanup(ai.close)
            ai.sendall(b"For")
            self.assertEqual(read_line(ai, timeout=0.2), "<timeout>")
            ai.sendall(b"ward\n")
            self.assertEqual(read_line(ai, timeout=1.0), "ok")
            server.assert_alive()

    def test_handshake_and_fragmented_command_in_same_stream(self):
        with ZappyServer(teams=("Team1", "teamB")) as server:
            client = server.connect_raw()
            self.addCleanup(client.close)
            client.sendall(b"Team1\nFor")
            self.assertRegex(read_line(client, timeout=1.0), r"^\d+$")
            self.assertEqual(read_line(client, timeout=1.0), f"{WIDTH} {HEIGHT}")
            self.assertEqual(read_line(client, timeout=0.2), "<timeout>")
            client.sendall(b"ward\n")
            self.assertEqual(read_line(client, timeout=1.0), "ok")
            server.assert_alive()

    def test_oversized_line_without_newline_does_not_crash(self):
        with ZappyServer() as server:
            client = server.connect_raw()
            self.addCleanup(client.close)
            client.sendall(b"A" * 65536)
            time.sleep(0.2)
            self.assertEqual(read_line(client, timeout=0.1), "<timeout>")
            client.sendall(b"\n")
            self.assertIn(read_line(client, timeout=1.0), {"ko", "<closed>"})
            server.assert_alive()

            healthy = server.connect_raw()
            healthy.close()

    def test_urandom_style_stream_after_auth_does_not_kill_server(self):
        with ZappyServer(clients=12) as server:
            ai, _, _ = server.connect_ai("teamA")
            self.addCleanup(ai.close)
            ai.sendall(os.urandom(262144))
            time.sleep(0.5)
            read_available(ai, total_timeout=0.2)
            server.assert_alive()

            healthy = server.connect_raw()
            healthy.close()

    def test_team_then_urandom_without_reading_handshake_does_not_kill_server(self):
        with ZappyServer(clients=12, teams=("team1", "teamB")) as server:
            client = server.connect_raw()
            self.addCleanup(client.close)
            client.sendall(b"team1\n" + os.urandom(262144))
            time.sleep(0.5)
            read_available(client, total_timeout=0.2)
            server.assert_alive()

            healthy = server.connect_raw()
            healthy.close()

    def test_parallel_noisy_clients_do_not_block_clean_client(self):
        with ZappyServer(clients=32) as server:
            noisy = []
            self.addCleanup(lambda: [sock.close() for sock in noisy])
            for index in range(16):
                sock = server.connect_raw()
                noisy.append(sock)
                if index % 2 == 0:
                    sock.sendall(b"teamA\n" + os.urandom(8192))
                else:
                    sock.sendall((b"unknown-team\n" * 64) + b"B" * 4096)

            time.sleep(0.4)
            server.assert_alive()
            clean = server.connect_raw()
            self.addCleanup(clean.close)
            send_line(clean, "teamB")
            self.assertRegex(read_line(clean, timeout=1.0), r"^\d+$")
            self.assertEqual(read_line(clean, timeout=1.0), f"{WIDTH} {HEIGHT}")


if __name__ == "__main__":
    unittest.main(verbosity=2)
