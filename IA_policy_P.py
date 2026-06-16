#!/usr/bin/env python3

import argparse
import random

from helpers_net import Connection, handshake
from helpers_parsing import get_response, parsing_vision, parsing_stuff
from helpers_together_inv import broadcast_inventory
from search_mode import search_food, search_food_or_t, search_level1, search_level2


def Heuristique_policyV1(state):

    food = state["stuff"]["food"]
    if food < 3:
        state["feeding"] = True
    elif food >= 8:
        state["feeding"] = False

    if food < 3:
        return search_food(state)

    if state.get("feeding"):
        return search_food_or_t(state)

    if state["level"] == 1:
        return search_level1(state)

    if state["level"] == 2:
        return search_level2(state)

    return random.choice(["Forward", "Forward", "Left", "Right"])


def run(host, port, team_name, player_id):
    conn = Connection(host, port)
    slots, width, height = handshake(conn, team_name)
    print(f"[handshake] slots restants={slots}  map={width}x{height}")
    last = ""
    action_len = 0
    level = 1
    child = []
    state = {"width": width, "height": height, "vision": [], "id": player_id, "last": last, "stuff": [],"level": level, "broadcast": [], "team": {}, "child": False, "feeding": False}
    try:
        while True:
            conn.send("Look")
            look_line = get_response(conn, state)
            if look_line == "dead":
                print("[mort] l'entité est morte de faim.")
                break
            state["vision"] = parsing_vision(look_line)

            if action_len % 10 == 0:
                conn.send("Inventory")
                stuff_line = get_response(conn, state)
                state["stuff"] = parsing_stuff(stuff_line)
            if action_len % 5 == 0:
                conn.send(broadcast_inventory(state))
                get_response(conn, state)


            command = Heuristique_policyV1(state)
            state["last"] = command
            action_len = action_len + 1


            conn.send(command)
            if command == "Incantation":
                print(f"vision={state['vision']}  ->  Incantation (fire-and-forget)")
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
