#!/usr/bin/env python3
"""
Advanced Zappy protocol tests: incantation, player death, ejection, broadcast
direction and resource GUI events.

Two suites:

  * Standalone (`*Tests`)         -- run against the local server only and assert
                                     exact protocol behaviour described in the
                                     subject / GUI protocol document.
  * Comparison (`Official*Tests`) -- run the SAME scenario against both the local
                                     server and `zappy_server_official` and assert
                                     identical observable behaviour.

Key facts established empirically against the reference server (see calibration):

  * The official server REQUIRES 10 <= width/height <= 42, so every scenario that
    needs two players on the same tile or a known resource on a tile is set up
    with a small navigator that drives players to chosen coordinates on a 10x10
    map (instead of relying on a 1x1 map, which the official server rejects).
  * The official server numbers players from #0, the local one from #1, so the
    GUI player reference is always discovered from the `pnw #n` event rather than
    hard-coded.
  * Movement convention (identical on both servers):
        orientation 1=N -> (0,-1), 2=E -> (+1,0), 3=S -> (0,+1), 4=W -> (-1,0)
        `Right` rotates 1->2->3->4->1.
  * Incantation 1->2 on a tile holding a linemate yields, on the official server:
        AI : "Elevation underway" then "Current level: 2"
        GUI: pic X Y 1 #n ... / plv #n 2 / pie X Y 1

f=500 is used for action-heavy scenarios (incantation = 0.6s, death = ~2.5s);
navigation on a 10x10 map completes well within the starvation budget.
"""
import os
import re
import socket
import subprocess
import time
import unittest
from contextlib import closing
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
OFFICIAL_SERVER = Path(os.environ.get("OFFICIAL_SERVER_BIN", ROOT / "zappy_server_official")).resolve()
_BUILD_SERVER = ROOT / "build" / "server-tests" / "zappy_server"
MINE_SERVER = Path(os.environ.get("MINE_SERVER_BIN", ROOT / "zappy_server")).resolve()
LOCAL_SERVER = Path(os.environ.get(
    "ZAPPY_SERVER_BIN", _BUILD_SERVER if _BUILD_SERVER.exists() else MINE_SERVER)).resolve()

WIDTH = 10
HEIGHT = 10
FREQ = 500             # death tests: 10 food * 126/500 = 2.52s to starve
# Navigation-heavy scenarios run at a lower frequency so the player cannot
# starve mid-setup (10 food * 126/100 = 12.6s budget); this isolates the
# feature under test from incidental starvation during navigation.
NAV_FREQ = 100
INCANT_TIMEOUT = 5.0   # incantation = 300/100 = 3s at NAV_FREQ, generous margin
DEATH_TIMEOUT = 6.0

# Resource name -> q index (q0=food .. q6=thystame). In a "bct X Y q0..q6" line
# the value for resource k is at split() index 3 + k.
RESOURCE_INDEX = {
    "food": 0, "linemate": 1, "deraumere": 2, "sibur": 3,
    "mendiane": 4, "phiras": 5, "thystame": 6,
}
DELTA = {1: (0, -1), 2: (1, 0), 3: (0, 1), 4: (-1, 0)}


# ---------------------------------------------------------------------------
# Socket helpers
# ---------------------------------------------------------------------------

def find_free_port():
    with closing(socket.socket(socket.AF_INET, socket.SOCK_STREAM)) as s:
        s.bind(("127.0.0.1", 0))
        return s.getsockname()[1]


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
    return f"<timeout:{data.decode(errors='replace')}>" if data else "<timeout>"


def read_available(sock, idle_timeout=0.05, total_timeout=1.0, limit=2000):
    lines = []
    deadline = time.monotonic() + total_timeout
    while len(lines) < limit:
        remaining = deadline - time.monotonic()
        if remaining <= 0:
            break
        line = read_line(sock, timeout=min(idle_timeout, remaining))
        if line.startswith("<timeout") or line == "<closed>":
            break
        lines.append(line)
    return lines


def wait_for_pattern(sock, pattern, timeout):
    """Read lines until one fully matches `pattern`. Returns the line or None."""
    deadline = time.monotonic() + timeout
    while time.monotonic() < deadline:
        remaining = deadline - time.monotonic()
        if remaining <= 0:
            break
        line = read_line(sock, timeout=min(0.5, remaining))
        if line == "<closed>":
            return None
        if line.startswith("<timeout"):
            continue
        if re.fullmatch(pattern, line):
            return line
    return None


def send_line(sock, text):
    try:
        sock.sendall((text + "\n").encode())
    except OSError:
        pass


def drain(sock, settle=0.0):
    if settle:
        time.sleep(settle)
    return read_available(sock, total_timeout=0.4)


# ---------------------------------------------------------------------------
# Server process
# ---------------------------------------------------------------------------

class ZappyServer:
    def __init__(self, *, width=WIDTH, height=HEIGHT, clients=4, freq=FREQ,
                 teams=("teamA", "teamB"), binary=None):
        self.binary = Path(binary or LOCAL_SERVER)
        self.width = width
        self.height = height
        self.clients = clients
        self.freq = freq
        self.teams = teams
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
            args, cwd=ROOT, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True
        )
        deadline = time.monotonic() + 2.0
        while time.monotonic() < deadline:
            if self.proc.poll() is not None:
                stderr = (self.proc.stderr.read() or "").strip()
                raise AssertionError(f"{self.binary.name} exited early: {stderr}")
            try:
                with socket.create_connection(("127.0.0.1", self.port), timeout=0.05) as s:
                    if read_line(s, timeout=0.2) == "WELCOME":
                        return self
            except OSError:
                time.sleep(0.02)
        raise AssertionError(f"{self.binary.name} did not accept connections")

    def __exit__(self, *_):
        if self.proc and self.proc.poll() is None:
            self.proc.terminate()
            try:
                self.proc.wait(timeout=1)
            except subprocess.TimeoutExpired:
                self.proc.kill()
                self.proc.wait(timeout=1)
        if self.proc:
            for s in (self.proc.stdout, self.proc.stderr):
                if s:
                    s.close()

    def connect_raw(self):
        sock = socket.create_connection(("127.0.0.1", self.port), timeout=2)
        welcome = read_line(sock)
        assert welcome == "WELCOME", f"expected WELCOME, got {welcome!r}"
        return sock

    def connect_gui(self):
        sock = self.connect_raw()
        send_line(sock, "GRAPHIC")
        gui_drain_until_sgt(sock)
        drain(sock)
        return sock

    def connect_ai(self, team="teamA"):
        sock = self.connect_raw()
        send_line(sock, team)
        slots = read_line(sock)
        size = read_line(sock)
        return sock, slots, size

    def connect_ai_tracked(self, gui, team="teamA"):
        """Connect an AI and discover its GUI player reference via the pnw event."""
        sock, slots, size = self.connect_ai(team)
        ref = None
        for line in drain(gui, settle=0.1):
            m = re.match(r"pnw (#\d+) ", line)
            if m:
                ref = m.group(1)
        return sock, ref, slots, size


def gui_drain_until_sgt(sock):
    lines = []
    for _ in range(2000):
        line = read_line(sock, timeout=2.0)
        lines.append(line)
        if line.startswith("sgt ") or line == "<closed>":
            break
    return lines


# ---------------------------------------------------------------------------
# Navigation helpers (drive an AI to chosen coordinates on a 10x10 map)
# ---------------------------------------------------------------------------

def get_pos(gui, ref):
    send_line(gui, "ppo " + ref)
    parts = read_line(gui).split()
    if len(parts) < 5 or parts[0] != "ppo":
        return None
    return int(parts[2]), int(parts[3]), int(parts[4])


def _turn_cmds(current, want):
    diff = (want - current) % 4
    return {0: [], 1: ["Right"], 2: ["Right", "Right"], 3: ["Left"]}[diff]


def navigate(ai, gui, ref, tx, ty, w=WIDTH, h=HEIGHT):
    """Greedy feedback navigation. Returns True if (tx,ty) reached."""
    for _ in range(2 * (w + h) + 10):
        pos = get_pos(gui, ref)
        if pos is None:
            return False
        x, y, o = pos
        if x == tx and y == ty:
            return True
        if x != tx:
            want = 2 if (tx - x) % w <= (x - tx) % w else 4
        else:
            want = 3 if (ty - y) % h <= (y - ty) % h else 1
        for cmd in _turn_cmds(o, want):
            send_line(ai, cmd)
            if read_line(ai, timeout=1.5) != "ok":
                return False
        send_line(ai, "Forward")
        if read_line(ai, timeout=1.5) != "ok":
            return False
        drain(gui)
    return False


def read_map(gui, w=WIDTH, h=HEIGHT):
    send_line(gui, "mct")
    tiles = {}
    for _ in range(w * h):
        parts = read_line(gui, timeout=1.0).split()
        if len(parts) >= 10 and parts[0] == "bct":
            tiles[(int(parts[1]), int(parts[2]))] = [int(v) for v in parts[3:10]]
    return tiles


def find_resource_tile(gui, resource, w=WIDTH, h=HEIGHT):
    """Return (x, y) of a tile that holds >=1 of `resource`, or None."""
    idx = RESOURCE_INDEX[resource]
    for (x, y), quantities in read_map(gui, w, h).items():
        if quantities[idx] >= 1:
            return x, y
    return None


def tile_resource_count(gui, x, y, resource):
    """Query a tile, skipping any pending GUI events to find its `bct` reply."""
    drain(gui)
    send_line(gui, f"bct {x} {y}")
    deadline = time.monotonic() + 1.5
    expected = re.compile(rf"bct {x} {y}(?: \d+){{7}}")
    while time.monotonic() < deadline:
        line = read_line(gui, timeout=0.5)
        if line.startswith("<timeout") or line == "<closed>":
            continue
        if expected.fullmatch(line):
            return int(line.split()[3 + RESOURCE_INDEX[resource]])
    return None


# ===========================================================================
# Standalone suite -- local server, exact protocol assertions
# ===========================================================================

@unittest.skipUnless(LOCAL_SERVER.exists(), f"local server not found: {LOCAL_SERVER}")
class IncantationTests(unittest.TestCase):
    """Level 1->2 incantation, set up deterministically via navigation."""

    def _player_on_linemate(self):
        server = ZappyServer(freq=NAV_FREQ).__enter__()
        self.addCleanup(server.__exit__, None, None, None)
        gui = server.connect_gui()
        self.addCleanup(gui.close)
        ai, ref, _, _ = server.connect_ai_tracked(gui)
        self.addCleanup(ai.close)
        self.assertIsNotNone(ref, "could not discover player ref from pnw")
        target = find_resource_tile(gui, "linemate")
        self.assertIsNotNone(target, "no linemate tile found on map")
        self.assertTrue(navigate(ai, gui, ref, *target),
                        f"navigation to {target} failed")
        self.assertGreaterEqual(tile_resource_count(gui, *target, "linemate"), 1,
                                "linemate disappeared from target tile")
        drain(gui)
        return server, gui, ai, ref, target

    def test_success_ai_messages(self):
        _, _, ai, _, _ = self._player_on_linemate()
        send_line(ai, "Incantation")
        self.assertEqual(read_line(ai, timeout=INCANT_TIMEOUT), "Elevation underway")
        self.assertEqual(read_line(ai, timeout=INCANT_TIMEOUT), "Current level: 2")

    def test_success_gui_events(self):
        _, gui, ai, _, _ = self._player_on_linemate()
        send_line(ai, "Incantation")
        self.assertEqual(read_line(ai, timeout=INCANT_TIMEOUT), "Elevation underway")
        self.assertEqual(read_line(ai, timeout=INCANT_TIMEOUT), "Current level: 2")
        events = drain(gui, settle=0.1)
        self.assertTrue(any(re.fullmatch(r"pic \d+ \d+ \d+( #\d+)+", e) for e in events),
                        f"missing pic event: {events}")
        self.assertTrue(any(re.fullmatch(r"pie \d+ \d+ \d+", e) for e in events),
                        f"missing pie event: {events}")
        self.assertTrue(any(re.fullmatch(r"plv #\d+ 2", e) for e in events),
                        f"missing plv level-up: {events}")

    def test_success_consumes_linemate(self):
        _, gui, ai, _, target = self._player_on_linemate()
        before = tile_resource_count(gui, *target, "linemate")
        send_line(ai, "Incantation")
        read_line(ai, timeout=INCANT_TIMEOUT)
        read_line(ai, timeout=INCANT_TIMEOUT)
        time.sleep(0.1)
        after = tile_resource_count(gui, *target, "linemate")
        self.assertLess(after, before,
                        f"linemate should be consumed: {before} -> {after}")

    def test_level2_single_player_fails(self):
        """Level 2->3 needs two players; a lone player must get ko."""
        _, _, ai, _, _ = self._player_on_linemate()
        send_line(ai, "Incantation")
        self.assertEqual(read_line(ai, timeout=INCANT_TIMEOUT), "Elevation underway")
        self.assertEqual(read_line(ai, timeout=INCANT_TIMEOUT), "Current level: 2")
        send_line(ai, "Incantation")
        self.assertEqual(read_line(ai, timeout=INCANT_TIMEOUT), "ko",
                         "level 2->3 with a single player must fail")


@unittest.skipUnless(LOCAL_SERVER.exists(), f"local server not found: {LOCAL_SERVER}")
class PlayerDeathTests(unittest.TestCase):
    def test_gui_receives_pdi(self):
        with ZappyServer(freq=FREQ) as server:
            gui = server.connect_gui()
            self.addCleanup(gui.close)
            ai, _, _ = server.connect_ai("teamA")
            self.addCleanup(ai.close)
            self.assertIsNotNone(wait_for_pattern(gui, r"pdi #\d+", DEATH_TIMEOUT),
                                 "expected pdi #n before timeout")

    def test_ai_receives_dead(self):
        with ZappyServer(freq=FREQ) as server:
            ai, _, _ = server.connect_ai("teamA")
            self.addCleanup(ai.close)
            self.assertEqual(wait_for_pattern(ai, r"dead", DEATH_TIMEOUT), "dead",
                             "starving AI should receive 'dead'")

    def test_dead_player_ppo_is_sbp(self):
        with ZappyServer(freq=FREQ) as server:
            gui = server.connect_gui()
            self.addCleanup(gui.close)
            ai, _, _ = server.connect_ai("teamA")
            self.addCleanup(ai.close)
            pdi = wait_for_pattern(gui, r"pdi #\d+", DEATH_TIMEOUT)
            self.assertIsNotNone(pdi, "expected pdi event")
            ref = re.search(r"(#\d+)", pdi).group(1)
            drain(gui)
            send_line(gui, f"ppo {ref}")
            self.assertEqual(read_line(gui, timeout=1.0), "sbp",
                             "querying a dead player should return sbp")


@unittest.skipUnless(LOCAL_SERVER.exists(), f"local server not found: {LOCAL_SERVER}")
class EjectionTests(unittest.TestCase):
    """Two players co-located at (0,0) via navigation, then Eject."""

    def _two_players_colocated(self):
        server = ZappyServer(freq=NAV_FREQ).__enter__()
        self.addCleanup(server.__exit__, None, None, None)
        gui = server.connect_gui()
        self.addCleanup(gui.close)
        ai1, ref1, _, _ = server.connect_ai_tracked(gui)
        self.addCleanup(ai1.close)
        ai2, ref2, _, _ = server.connect_ai_tracked(gui)
        self.addCleanup(ai2.close)
        self.assertIsNotNone(ref1)
        self.assertIsNotNone(ref2)
        self.assertTrue(navigate(ai1, gui, ref1, 0, 0), "ai1 navigation failed")
        self.assertTrue(navigate(ai2, gui, ref2, 0, 0), "ai2 navigation failed")
        drain(gui)
        return server, gui, ai1, ref1, ai2, ref2

    def test_pex_sent_to_gui(self):
        _, gui, ai1, _, _, _ = self._two_players_colocated()
        send_line(ai1, "Eject")
        self.assertEqual(read_line(ai1, timeout=1.5), "ok")
        events = drain(gui, settle=0.1)
        self.assertTrue(any(re.fullmatch(r"pex #\d+", e) for e in events),
                        f"missing pex event: {events}")

    def test_ejected_player_receives_direction(self):
        _, _, ai1, _, ai2, _ = self._two_players_colocated()
        send_line(ai1, "Eject")
        self.assertEqual(read_line(ai1, timeout=1.5), "ok")
        line = read_line(ai2, timeout=1.5)
        m = re.fullmatch(r"eject: (\d+)", line)
        self.assertIsNotNone(m, f"ejected player should get 'eject: K', got {line!r}")
        self.assertIn(int(m.group(1)), range(0, 9))

    def test_eject_alone_returns_ok(self):
        # The reference server answers "ok" to Eject even with nobody else on
        # the tile (verified against zappy_server_official).
        with ZappyServer(freq=FREQ, clients=1, teams=("solo",)) as server:
            ai, _, _ = server.connect_ai("solo")
            self.addCleanup(ai.close)
            send_line(ai, "Eject")
            self.assertEqual(read_line(ai, timeout=1.5), "ok")


@unittest.skipUnless(LOCAL_SERVER.exists(), f"local server not found: {LOCAL_SERVER}")
class BroadcastDirectionTests(unittest.TestCase):
    def _two_players_colocated(self):
        server = ZappyServer(freq=NAV_FREQ).__enter__()
        self.addCleanup(server.__exit__, None, None, None)
        gui = server.connect_gui()
        self.addCleanup(gui.close)
        ai1, ref1, _, _ = server.connect_ai_tracked(gui)
        self.addCleanup(ai1.close)
        ai2, ref2, _, _ = server.connect_ai_tracked(gui)
        self.addCleanup(ai2.close)
        self.assertTrue(navigate(ai1, gui, ref1, 0, 0), "ai1 navigation failed")
        self.assertTrue(navigate(ai2, gui, ref2, 0, 0), "ai2 navigation failed")
        drain(gui)
        return server, gui, ai1, ai2

    def test_same_tile_direction_is_zero(self):
        _, _, ai1, ai2 = self._two_players_colocated()
        send_line(ai1, "Broadcast ping")
        self.assertEqual(read_line(ai1, timeout=1.5), "ok")
        self.assertEqual(read_line(ai2, timeout=1.5), "message 0, ping",
                         "broadcast from the same tile must arrive as direction 0")

    def test_message_format_and_range(self):
        with ZappyServer(freq=FREQ) as server:
            ai1, _, _ = server.connect_ai("teamA")
            self.addCleanup(ai1.close)
            ai2, _, _ = server.connect_ai("teamA")
            self.addCleanup(ai2.close)
            send_line(ai1, "Broadcast hello-zappy")
            self.assertEqual(read_line(ai1, timeout=1.5), "ok")
            line = read_line(ai2, timeout=1.5)
            m = re.fullmatch(r"message (\d+), hello-zappy", line)
            self.assertIsNotNone(m, f"bad broadcast format: {line!r}")
            self.assertIn(int(m.group(1)), range(0, 9))

    def test_gui_receives_pbc(self):
        with ZappyServer(freq=FREQ) as server:
            gui = server.connect_gui()
            self.addCleanup(gui.close)
            ai, _, _ = server.connect_ai("teamA")
            self.addCleanup(ai.close)
            drain(gui)
            send_line(ai, "Broadcast gui-bc")
            self.assertEqual(read_line(ai, timeout=1.5), "ok")
            events = drain(gui, settle=0.1)
            self.assertTrue(any(re.fullmatch(r"pbc #\d+ gui-bc", e) for e in events),
                            f"missing pbc event: {events}")


@unittest.skipUnless(LOCAL_SERVER.exists(), f"local server not found: {LOCAL_SERVER}")
class ResourceEventTests(unittest.TestCase):
    def _player_on_linemate(self):
        server = ZappyServer(freq=NAV_FREQ).__enter__()
        self.addCleanup(server.__exit__, None, None, None)
        gui = server.connect_gui()
        self.addCleanup(gui.close)
        ai, ref, _, _ = server.connect_ai_tracked(gui)
        self.addCleanup(ai.close)
        target = find_resource_tile(gui, "linemate")
        self.assertIsNotNone(target, "no linemate tile found")
        self.assertTrue(navigate(ai, gui, ref, *target), "navigation failed")
        drain(gui)
        return server, gui, ai, target

    def test_take_linemate_ok_and_pgt(self):
        _, gui, ai, _ = self._player_on_linemate()
        send_line(ai, "Take linemate")
        self.assertEqual(read_line(ai, timeout=1.5), "ok")
        events = drain(gui, settle=0.1)
        self.assertTrue(any(re.fullmatch(r"pgt #\d+ 1", e) for e in events),
                        f"missing pgt #n 1 (linemate index 1): {events}")

    def test_set_linemate_pdr(self):
        _, gui, ai, _ = self._player_on_linemate()
        send_line(ai, "Take linemate")
        self.assertEqual(read_line(ai, timeout=1.5), "ok")
        drain(gui)
        send_line(ai, "Set linemate")
        self.assertEqual(read_line(ai, timeout=1.5), "ok")
        events = drain(gui, settle=0.1)
        self.assertTrue(any(re.fullmatch(r"pdr #\d+ 1", e) for e in events),
                        f"missing pdr #n 1: {events}")

    def test_take_increments_inventory(self):
        _, _, ai, _ = self._player_on_linemate()
        send_line(ai, "Take linemate")
        self.assertEqual(read_line(ai, timeout=1.5), "ok")
        send_line(ai, "Inventory")
        inv = read_line(ai, timeout=1.5)
        m = re.search(r"linemate (\d+)", inv)
        self.assertIsNotNone(m, f"bad inventory format: {inv!r}")
        self.assertGreaterEqual(int(m.group(1)), 1, f"linemate not in inventory: {inv}")

    def test_set_missing_resource_ko(self):
        with ZappyServer(freq=FREQ) as server:
            ai, _, _ = server.connect_ai("teamA")
            self.addCleanup(ai.close)
            send_line(ai, "Set thystame")
            self.assertEqual(read_line(ai, timeout=1.5), "ko",
                             "setting a resource you don't hold must be ko")


# ===========================================================================
# Comparison suite -- local server vs official server, identical behaviour
# ===========================================================================

def run_on_both(callback, **server_kwargs):
    results = {}
    for label, binary in (("official", OFFICIAL_SERVER), ("mine", MINE_SERVER)):
        with ZappyServer(binary=binary, **server_kwargs) as server:
            results[label] = callback(server)
    return results


@unittest.skipUnless(
    OFFICIAL_SERVER.exists() and MINE_SERVER.exists(),
    "both official and local server binaries are required",
)
class OfficialAdvancedComparisonTests(unittest.TestCase):
    maxDiff = None

    def assert_same(self, results):
        self.assertEqual(results["mine"], results["official"], results)

    def _tracked_ai(self, server):
        gui = server.connect_gui()
        ai, ref, _, _ = server.connect_ai_tracked(gui)
        drain(gui)
        return gui, ai, ref

    # --- Incantation ---

    def test_incantation_ai_response_matches(self):
        def scenario(server):
            gui = server.connect_gui()
            ai, ref, _, _ = server.connect_ai_tracked(gui)
            target = find_resource_tile(gui, "linemate")
            nav = navigate(ai, gui, ref, *target) if target else False
            drain(gui)
            send_line(ai, "Incantation")
            first = read_line(ai, timeout=INCANT_TIMEOUT)
            second = read_line(ai, timeout=INCANT_TIMEOUT)
            ai.close()
            gui.close()
            return {"navigated": nav, "first": first, "second": second}

        self.assert_same(run_on_both(scenario, freq=NAV_FREQ))

    def test_incantation_gui_events_match(self):
        def scenario(server):
            gui = server.connect_gui()
            ai, ref, _, _ = server.connect_ai_tracked(gui)
            target = find_resource_tile(gui, "linemate")
            navigate(ai, gui, ref, *target)
            drain(gui)
            send_line(ai, "Incantation")
            read_line(ai, timeout=INCANT_TIMEOUT)
            read_line(ai, timeout=INCANT_TIMEOUT)
            events = drain(gui, settle=0.1)
            ai.close()
            gui.close()
            return {
                "has_valid_pic": any(re.fullmatch(r"pic \d+ \d+ \d+( #\d+)+", e) for e in events),
                "has_valid_pie": any(re.fullmatch(r"pie \d+ \d+ \d+", e) for e in events),
                "has_plv_2": any(re.fullmatch(r"plv #\d+ 2", e) for e in events),
            }

        self.assert_same(run_on_both(scenario, freq=NAV_FREQ))

    def test_incantation_consumes_linemate_matches(self):
        def scenario(server):
            gui = server.connect_gui()
            ai, ref, _, _ = server.connect_ai_tracked(gui)
            target = find_resource_tile(gui, "linemate")
            navigate(ai, gui, ref, *target)
            before = tile_resource_count(gui, *target, "linemate")
            send_line(ai, "Incantation")
            read_line(ai, timeout=INCANT_TIMEOUT)
            read_line(ai, timeout=INCANT_TIMEOUT)
            time.sleep(0.1)
            after = tile_resource_count(gui, *target, "linemate")
            ai.close()
            gui.close()
            return {"linemate_consumed": after < before}

        self.assert_same(run_on_both(scenario, freq=NAV_FREQ))

    def test_incantation_level2_single_player_matches(self):
        def scenario(server):
            gui = server.connect_gui()
            ai, ref, _, _ = server.connect_ai_tracked(gui)
            target = find_resource_tile(gui, "linemate")
            navigate(ai, gui, ref, *target)
            send_line(ai, "Incantation")
            read_line(ai, timeout=INCANT_TIMEOUT)
            read_line(ai, timeout=INCANT_TIMEOUT)
            send_line(ai, "Incantation")
            result = read_line(ai, timeout=INCANT_TIMEOUT)
            ai.close()
            gui.close()
            return {"level2_single": result}

        self.assert_same(run_on_both(scenario, freq=NAV_FREQ))

    # --- Player death ---

    def test_death_pdi_matches(self):
        def scenario(server):
            gui = server.connect_gui()
            ai, _, _ = server.connect_ai("teamA")
            matched = wait_for_pattern(gui, r"pdi #\d+", DEATH_TIMEOUT)
            ai.close()
            gui.close()
            return {"pdi_format_ok": bool(matched and re.fullmatch(r"pdi #\d+", matched))}

        self.assert_same(run_on_both(scenario))

    def test_death_ai_dead_matches(self):
        def scenario(server):
            ai, _, _ = server.connect_ai("teamA")
            matched = wait_for_pattern(ai, r"dead", DEATH_TIMEOUT)
            ai.close()
            return {"received_dead": matched == "dead"}

        self.assert_same(run_on_both(scenario))

    # --- Ejection ---

    def _colocate(self, server):
        gui = server.connect_gui()
        ai1, ref1, _, _ = server.connect_ai_tracked(gui)
        ai2, ref2, _, _ = server.connect_ai_tracked(gui)
        navigate(ai1, gui, ref1, 0, 0)
        navigate(ai2, gui, ref2, 0, 0)
        drain(gui)
        return gui, ai1, ai2

    def test_ejection_matches(self):
        def scenario(server):
            gui, ai1, ai2 = self._colocate(server)
            send_line(ai1, "Eject")
            ejector = read_line(ai1, timeout=1.5)
            ejected = read_line(ai2, timeout=1.5)
            events = drain(gui, settle=0.1)
            ai1.close()
            ai2.close()
            gui.close()
            return {
                "ejector_response": ejector,
                "ejected_format_ok": bool(re.fullmatch(r"eject: \d+", ejected)),
                "ejected_k_in_range": bool(re.fullmatch(r"eject: [1-8]", ejected)),
                "gui_has_pex": any(re.fullmatch(r"pex #\d+", e) for e in events),
            }

        self.assert_same(run_on_both(scenario, freq=NAV_FREQ))

    def test_eject_alone_matches(self):
        def scenario(server):
            ai, _, _ = server.connect_ai("solo")
            send_line(ai, "Eject")
            resp = read_line(ai, timeout=1.5)
            ai.close()
            return {"eject_alone": resp}

        self.assert_same(run_on_both(scenario, clients=1, teams=("solo",)))

    def test_fork_then_eject_events_match(self):
        """Fork lays an egg, then a co-located player ejects: compare the set of
        GUI event types (pfk / enw / pex / edi) emitted by both servers."""
        def scenario(server):
            gui, ai1, ai2 = self._colocate(server)
            send_line(ai1, "Fork")
            fork_resp = read_line(ai1, timeout=1.5)
            fork_events = drain(gui, settle=0.1)
            send_line(ai2, "Eject")
            eject_resp = read_line(ai2, timeout=1.5)
            eject_events = drain(gui, settle=0.1)
            ai1.close()
            ai2.close()
            gui.close()
            return {
                "fork_resp": fork_resp,
                "eject_resp": eject_resp,
                "fork_has_pfk": any(e.startswith("pfk ") for e in fork_events),
                "fork_has_enw": any(e.startswith("enw ") for e in fork_events),
                "eject_has_pex": any(e.startswith("pex ") for e in eject_events),
                "eject_has_edi": any(e.startswith("edi ") for e in eject_events),
            }

        self.assert_same(run_on_both(scenario, clients=6, freq=NAV_FREQ))

    # --- Broadcast ---

    def test_broadcast_same_tile_matches(self):
        def scenario(server):
            gui, ai1, ai2 = self._colocate(server)
            send_line(ai1, "Broadcast cmp-msg")
            sender = read_line(ai1, timeout=1.5)
            received = read_line(ai2, timeout=1.5)
            ai1.close()
            ai2.close()
            gui.close()
            return {"sender": sender, "received": received}

        self.assert_same(run_on_both(scenario, freq=NAV_FREQ))

    def test_broadcast_gui_pbc_matches(self):
        def scenario(server):
            gui = server.connect_gui()
            ai, _, _ = server.connect_ai("teamA")
            drain(gui)
            send_line(ai, "Broadcast cmp-gui")
            resp = read_line(ai, timeout=1.5)
            events = drain(gui, settle=0.1)
            ai.close()
            gui.close()
            return {
                "ai_response": resp,
                "has_pbc": any(re.fullmatch(r"pbc #\d+ cmp-gui", e) for e in events),
            }

        self.assert_same(run_on_both(scenario))

    # --- Resource events ---

    def test_take_linemate_matches(self):
        def scenario(server):
            gui = server.connect_gui()
            ai, ref, _, _ = server.connect_ai_tracked(gui)
            target = find_resource_tile(gui, "linemate")
            navigate(ai, gui, ref, *target)
            drain(gui)
            send_line(ai, "Take linemate")
            resp = read_line(ai, timeout=1.5)
            events = drain(gui, settle=0.1)
            ai.close()
            gui.close()
            return {
                "take_response": resp,
                "has_pgt_1": any(re.fullmatch(r"pgt #\d+ 1", e) for e in events),
            }

        self.assert_same(run_on_both(scenario, freq=NAV_FREQ))

    def test_set_linemate_matches(self):
        def scenario(server):
            gui = server.connect_gui()
            ai, ref, _, _ = server.connect_ai_tracked(gui)
            target = find_resource_tile(gui, "linemate")
            navigate(ai, gui, ref, *target)
            send_line(ai, "Take linemate")
            read_line(ai, timeout=1.5)
            drain(gui)
            send_line(ai, "Set linemate")
            resp = read_line(ai, timeout=1.5)
            events = drain(gui, settle=0.1)
            ai.close()
            gui.close()
            return {
                "set_response": resp,
                "has_pdr_1": any(re.fullmatch(r"pdr #\d+ 1", e) for e in events),
            }

        self.assert_same(run_on_both(scenario, freq=NAV_FREQ))

    def test_set_missing_resource_matches(self):
        def scenario(server):
            ai, _, _ = server.connect_ai("teamA")
            send_line(ai, "Set thystame")
            resp = read_line(ai, timeout=1.5)
            ai.close()
            return {"set_missing": resp}

        self.assert_same(run_on_both(scenario))

    # --- Protocol edge cases ---

    def test_reserved_graphic_team_name_matches(self):
        def run_with_reserved_team(binary):
            port = find_free_port()
            proc = subprocess.Popen(
                [str(binary), "-p", str(port), "-x", str(WIDTH), "-y", str(HEIGHT),
                 "-n", "GRAPHIC", "teamA", "-c", "2", "-f", str(NAV_FREQ)],
                cwd=ROOT, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True
            )
            try:
                proc.wait(timeout=0.5)
                return {"state": "exited", "code": proc.returncode}
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
            "official": run_with_reserved_team(OFFICIAL_SERVER),
            "mine": run_with_reserved_team(MINE_SERVER),
        })

    def test_unknown_team_then_valid_team_same_socket_matches(self):
        def scenario(server):
            client = server.connect_raw()
            send_line(client, "ghost-team")
            first = read_line(client, timeout=1.0)
            send_line(client, "teamA")
            second = read_line(client, timeout=1.0)
            third = read_line(client, timeout=1.0)
            client.close()
            return {"first": first, "second": second, "third": third}

        self.assert_same(run_on_both(scenario))

    def test_ai_command_queue_overflow_matches(self):
        def scenario(server):
            ai, _, _ = server.connect_ai("teamA")
            for _ in range(12):
                send_line(ai, "Inventory")
            responses = [read_line(ai, timeout=2.0) for _ in range(12)]
            ai.close()
            categories = [
                "inventory" if line.startswith("[") else line
                for line in responses
            ]
            return {
                "count": len(responses),
                "ko_count": categories.count("ko"),
                "inventory_count": categories.count("inventory"),
                "timeout_count": categories.count("<timeout>"),
                "tail": categories[-3:],
            }

        self.assert_same(run_on_both(scenario, freq=NAV_FREQ))

    def test_ai_malformed_commands_match(self):
        commands = (
            "",
            "Forward extra",
            "Broadcast",
            "Broadcast ",
            "Take",
            "Take FOOD",
            "Take food extra",
            "Set",
            "Set food extra",
            "connect_nbr",
        )

        def scenario(server):
            ai, _, _ = server.connect_ai("teamA")
            result = {}
            for command in commands:
                send_line(ai, command)
                result[repr(command)] = read_line(ai, timeout=1.5)
            ai.close()
            return result

        self.assert_same(run_on_both(scenario, freq=NAV_FREQ))

    def test_gui_malformed_commands_match(self):
        commands = (
            "",
            "   ",
            "msz ",
            "msz extra",
            "mct extra",
            "tna extra",
            "bct",
            "bct 0",
            "bct 0 0 extra",
            "bct 999999999999999999999999 0",
            "bct 01 00",
            "ppo",
            "ppo #0 extra",
            "ppo 0",
            "ppo #-1",
            "plv #999999999999999999999999",
            "pin #999999999999999999999999",
            "sgt extra",
            "sst",
            "sst -1",
            "sst 0001",
            "sst 1000",
            "sst 1001",
        )

        def scenario(server):
            result = {}
            for command in commands:
                gui = server.connect_gui()
                drain(gui)
                send_line(gui, command)
                response = read_line(gui, timeout=1.0)
                result[repr(command)] = response.split(" ", 1)[0]
                if command == "bct 01 00":
                    result[repr(command) + ":shape_ok"] = bool(
                        re.fullmatch(r"bct 1 0(?: \d+){7}", response))
                if command == "sst 0001":
                    result[repr(command) + ":value"] = response
                gui.close()
            return result

        self.assert_same(run_on_both(scenario))

    def test_gui_player_queries_with_discovered_ref_match(self):
        def scenario(server):
            gui, ai, ref = self._tracked_ai(server)
            result = {"has_ref": ref is not None}
            for command in (f"ppo {ref}", f"plv {ref}", f"pin {ref}"):
                send_line(gui, command)
                result[command.split()[0]] = bool(read_line(gui, timeout=1.0)
                                                  .startswith(command.split()[0] + " "))
            ai.close()
            gui.close()
            return result

        self.assert_same(run_on_both(scenario))

    def test_partial_command_waits_for_newline_matches(self):
        def scenario(server):
            ai, _, _ = server.connect_ai("teamA")
            ai.sendall(b"Inven")
            before = read_line(ai, timeout=0.2)
            ai.sendall(b"tory\n")
            after = read_line(ai, timeout=1.5)
            ai.close()
            return {"before": before, "after_is_inventory": after.startswith("[")}

        self.assert_same(run_on_both(scenario, freq=NAV_FREQ))

    def test_client_disconnect_emits_pdi_and_releases_slot_matches(self):
        def scenario(server):
            gui = server.connect_gui()
            ai, ref, slots_before, _ = server.connect_ai_tracked(gui)
            ai.close()
            pdi = wait_for_pattern(gui, r"pdi #\d+", 2.0)
            replacement, slots_after, _ = server.connect_ai("teamA")
            replacement.close()
            gui.close()
            return {
                "had_ref": ref is not None,
                "slots_before": slots_before,
                "pdi_format_ok": bool(pdi and re.fullmatch(r"pdi #\d+", pdi)),
                "slots_after": slots_after,
            }

        self.assert_same(run_on_both(scenario))


if __name__ == "__main__":
    unittest.main(verbosity=2)
