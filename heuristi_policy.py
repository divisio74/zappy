#!/usr/bin/env python3

import argparse
import random
import socket
import math

class Connection:
    def __init__(self, host, port):
        self.sock = socket.create_connection((host, port))
        self.buffer = ""

    def readline(self):
        while "\n" not in self.buffer:
            chunk = self.sock.recv(4096)
            if not chunk:
                raise ConnectionError("serveur deconnecté")
            self.buffer += chunk.decode(errors="replace")
        line, self.buffer = self.buffer.split("\n", 1)
        return line

    def send(self, command):
        self.sock.sendall((command + "\n").encode())

    def close(self):
        self.sock.close()


def handshake(conn, team_name):
    welcome = conn.readline()
    if welcome != "WELCOME":
        raise ConnectionError(f"attendu 'WELCOME', recu {welcome!r}")

    conn.send(team_name)

    slots = int(conn.readline())
    width, height = map(int, conn.readline().split())
    return slots, width, height



def parsing_vision(line):
    inner = line.strip().lstrip("[").rstrip("]")
    return [cell.split() for cell in inner.split(",")]


def parsing_stuff(line):
    inner = line.strip().lstrip("[").rstrip("]")
    inv = {}
    for item in inner.split(","):
        parts = item.split()
        if len(parts) == 2:
            inv[parts[0]] = int(parts[1])
    return inv


def Heuristique_policyV1(state):

    vision = state["vision"]

    if state["stuff"]["food"] < 5:
        if vision and "food" in vision[0]:
            return "Take food"
        if state["last"] == "Right":
            return "Forward"
        if state["last"] == "Left":
            return "Forward"
        for i in range(len(vision)):
            y = int(math.sqrt(i))
            xx = y * (y + 1)
            x = i - xx
            if "food" in vision[xx]:
                return "Forward"
            if x < 0 and "food" in vision[i]:
                return "Left"
            if x > 0 and "food" in vision[i]:
                return "Right"

    if state["level"] == 0:
        if state["last"] == "Set linemate":
            return "Incantation"
        if state["stuff"]["linemate"] >= 1:
            return "Set linemate"
        if vision and "linemate" in vision[0]:
            return "Take linemate"
        if state["last"] == "Right":
            return "Forward"
        if state["last"] == "Left":
            return "Forward"
        for i in range(len(vision)):
            y = int(math.sqrt(i))
            xx = y * (y + 1)
            x = i - xx
            if "linemate" in vision[xx]:
                return "Forward"
            if x < 0 and "linemate" in vision[i]:
                return "Left"
            if x > 0 and "linemate" in vision[i]:
                return "Right"

    if state["level"] == 1:
        if len(state["childs"]) == 0:
            return "Fork"
        if len(state["childs"]) == 1 and state["stuff"]["deraumere"] >= 1 and state["stuff"]["sibur"] >= 1 and state["stuff"]["linemate"] >= 1:
            return "Broadcast duo_elevation"
        for i in range(len(state["childs"])):
            if state["broadcast"][i] == 0:
                return "Elevation"
        if "sibur" in vision[0]:
            return "Take sibur"
        if "deraumere" in vision[0]:
            return "Take deraumere"
        if "linemate" in vision[0]:
            return "Take linemate"
        if state["last"] == "Right":
            return "Forward"
        if state["last"] == "Left":
            return "Forward"
        for i in range(len(vision)):
            y = int(math.sqrt(i))
            xx = y * (y + 1)
            x = i - xx
            if "sibur" or "deraumere" or "linemate" in vision[xx]:
                return "Forward"
            if x < 0 and ("sibur" or "deraumere" or "linemate"  in vision[i]):
                return "Left"
            if x > 0 and ("sibur" or "deraumere" or "linemate"  in vision[i]):
                return "Right"


    return random.choice(["Forward", "Forward", "Left", "Right"])


def run(host, port, team_name):
    conn = Connection(host, port)
    slots, width, height = handshake(conn, team_name)
    print(f"[handshake] slots restants={slots}  map={width}x{height}")
    last = ""
    action_len = 0
    level = 0
    child = []
    state = {"width": width, "height": height, "vision": [], "last": last, "stuff": [],"level": level, "childs": [], "broadcast": []}
    try:
        while True:
            conn.send("Look")
            look_line = conn.readline()
            if look_line == "dead":
                print("[mort] l'entité est morte de faim.")
                break
            state["vision"] = parsing_vision(look_line)

            if action_len % 10 == 0:
                conn.send("Inventory")
                stuff_line = conn.readline()
                state["stuff"] = parsing_stuff(stuff_line)

            command = Heuristique_policyV1(state)
            state["last"] = command
            action_len = action_len + 1


            conn.send(command)
            reply = conn.readline()
            print(f"vision={state['vision']}  ->  {command}  ->  {reply!r}")
            if reply == "dead":
                print("[mort] l'entité est morte de faim.")
                break

            if reply == "Elevation underway":
                reply = conn.readline()
                state["level"] = state["level"] + 1

    except (ConnectionError, KeyboardInterrupt) as e:
        print(f"[fin] {e}")
    finally:
        conn.close()


def main():
    parser = argparse.ArgumentParser(description="AI client for zappy", add_help=False)
    parser.add_argument("-p", type=int, required=True, dest="port", help="port")
    parser.add_argument("-n", required=True, dest="name", help="nom d'equipe")
    parser.add_argument("-H", "-h", default="127.0.0.1", dest="host", help="machine")
    args = parser.parse_args()
    run(args.host, args.port, args.name)


if __name__ == "__main__":
    main()
