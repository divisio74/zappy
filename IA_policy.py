#!/usr/bin/env python3

import argparse
import os
import random
import subprocess
import sys

from IA_helpers_net import Connection, handshake
from IA_helpers_parsing import get_response, parsing_vision, parsing_stuff, read_incantation_result
from IA_helpers_together_inv import broadcast_inventory
from IA_search_mode import search_food_or_t, search_level1, search_level2, search_level3, search_level4, search_level5, search_level6, search_level7

def Heuristique_policyV1(state, player_id):

    vision = state["vision"]
    if vision and "food" in vision[0]:
        return "Take food"
    food = state["stuff"].get("food", 0)
    if food < 8:
        state["feeding"] = True
    elif food >= 16:
        state["feeding"] = False

    if state.get("feeding"):
        return search_food_or_t(state)

    if state["level"] == 1:
        return search_level1(state)

    if state["level"] == 2:
        return search_level2(state)

    if state["level"] == 3:
        return search_level3(state)

    if state["level"] == 4:
        return search_level4(state)

    if state["level"] == 5:
        return search_level5(state)

    if state["level"] == 6:
        return search_level6(state)

    if state["level"] == 7:
        return search_level7(state)

    return random.choice(["Forward", "Forward", "Left", "Right"])


def run(host, port, team_name, player_id):
    conn = Connection(host, port)
    slots, width, height = handshake(conn, team_name)
    print(f"[handshake] slots restants={slots}  map={width}x{height}")
    last = ""
    action_len = 0
    level = 1
    child = []
    state = {"width": width, "height": height, "vision": [], "id": player_id, "last": last, "stuff": {},"level": level, "broadcast": [], "team": {}, "spawned_ids": [], "feeding": False}
    try:
        while True:
            conn.send("Look")
            look_line = get_response(conn, state)
            if look_line == "dead":
                print("[mort] l'entité est morte de faim.")
                break
            state["vision"] = parsing_vision(look_line)

            conn.send("Inventory")
            stuff_line = get_response(conn, state)
            state["stuff"] = parsing_stuff(stuff_line)
            if action_len % 10 == 0:
                conn.send(broadcast_inventory(state))
                get_response(conn, state)


            command = Heuristique_policyV1(state, player_id)
            state["last"] = command
            action_len = action_len + 1


            conn.send(command)
            if command == "Incantation":
                res = read_incantation_result(conn, state)
                print(f"vision={state['vision']}  ->  Incantation  ->  {res!r}  (niveau={state['level']})")
                continue
            if command == "Fork":
                child_id = None
                if player_id == 1:
                    if 2 not in state["spawned_ids"]:
                        child_id = 2
                    elif 3 not in state["spawned_ids"]:
                        child_id = 3
                    elif 4 not in state["spawned_ids"]:
                        child_id = 4
                    elif 5 not in state["spawned_ids"]:
                        child_id = 5
                    elif 6 not in state["spawned_ids"]:
                        child_id = 6
                if child_id is not None:
                    subprocess.Popen([sys.executable, os.path.abspath(__file__),
                                      "-p", str(port), "-n", team_name,
                                      "-i", str(child_id), "-H", host])
                    print(f"[fork] spawn du joueur {child_id}")
                    state["spawned_ids"].append(child_id)
                get_response(conn, state)
                continue

            reply = get_response(conn, state)
            print(f"vision={state['vision']}  ->  {command}  ->  {reply!r}")
            if reply == "dead":
                print("[mort] l'entité est morte de faim.")
                break

    except (ConnectionError, KeyboardInterrupt) as e:
        print(f"[fin] {e}")
    finally:
        conn.close()


def main():
    parser = argparse.ArgumentParser(description="AI client for zappy", add_help=False)
    parser.add_argument("-i", type=int, required=True, dest="id", help="id du joueur")
    parser.add_argument("-p", type=int, required=True, dest="port", help="port")
    parser.add_argument("-n", required=True, dest="name", help="nom d'equipe")
    parser.add_argument("-H", "-h", default="127.0.0.1", dest="host", help="machine")
    args = parser.parse_args()
    run(args.host, args.port, args.name, args.id)


if __name__ == "__main__":
    main()
