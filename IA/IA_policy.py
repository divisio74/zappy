#!/usr/bin/env python3

import argparse
import os
import random
import subprocess
import sys

from IA_helpers_net import Connection, handshake
from IA_helpers_parsing import get_response, parsing_vision, parsing_stuff, read_incantation_result
from IA_helpers_together_inv import broadcast_inventory
from IA_navigation import update_nav
from IA_search_mode import search_food_or_t, search_level1, search_level2, search_level3, search_level4, search_level5, search_level6, search_level7

def refresh_view(conn, state):
    look_line = get_response(conn, state)
    if look_line == "dead":
        return False
    state["vision"] = parsing_vision(look_line)
    stuff_line = get_response(conn, state)
    if stuff_line == "dead":
        return False
    state["stuff"] = parsing_stuff(stuff_line)
    return True

def handle_fork(state, player_id, host, port, team_name):
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
        "-p", str(port), "-n", team_name, "-i", str(child_id), "-H", host])
        print(f"[fork] spawn du joueur {child_id}")
        state["spawned_ids"].append(child_id)
    return child_id


def Heuristique_policyV2(state, player_id):

    food = state["stuff"].get("food", 0)
    vision = state["vision"]
    if vision and "food" in vision[0] and food <= 24:
        return "Take food"
    rallying = (bool(vision) and vision[0].count("player") >= 5) or any(info.get("type") == "rally" for info in state["team"].values())
    if food < 10 or (food < 12 and not rallying):
        state["feeding"] = True
    elif food >= 20 or (rallying and food >= 8):
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
    print(f"[handshake] Validé : slots restants={slots}  map={width}x{height}")
    state = {"width": width, "height": height, "vision": [], "id": player_id, "last": "", "stuff": {}, "level": 1, "broadcast": [], "team": {}, "spawned_ids": [], "feeding": False}

    conn.send_batch(["Look", "Inventory"])
    if not refresh_view(conn, state):
        print("[mort] L'entité est morte de faim.")
        conn.close()
        return

    action_len = 0
    try:
        while True:
            command = Heuristique_policyV2(state, player_id)
            state["last"] = command
            update_nav(state, command)
            action_len = action_len + 1

            if command == "Incantation":
                conn.send_batch(["Incantation", "Look", "Inventory"])
                res = read_incantation_result(conn, state)
                print(f"Vision={state['vision']}  ->  Incantation  ->  {res!r}  (Niveau={state['level']})")
                if not refresh_view(conn, state):
                    print("[mort] L'entité est morte de faim.")
                    break
                continue

            if command == "Fork":
                conn.send_batch(["Fork", "Look", "Inventory"])
                get_response(conn, state)
                handle_fork(state, player_id, host, port, team_name)
                if not refresh_view(conn, state):
                    print("[mort] L'entité est morte de faim.")
                    break
                continue

            send_broadcast = (action_len % 10 == 0)
            batch = [broadcast_inventory(state)] if send_broadcast else []
            batch += [command, "Look", "Inventory"]
            conn.send_batch(batch)

            if send_broadcast:
                get_response(conn, state)
            reply = get_response(conn, state)
            if reply == "dead" or not refresh_view(conn, state):
                print("[mort] L'entité est morte de faim.")
                break

    except BrokenPipeError:
        print("[mort] L'entité est morte de faim.")
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
