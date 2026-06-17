#!/usr/bin/env python3

from IA_helpers_net import Team_Password

RECIPES = {
    1: (1, {"linemate": 1}),
    2: (2, {"linemate": 1, "deraumere": 1, "sibur": 1}),
    3: (2, {"linemate": 2, "sibur": 1, "phiras": 2}),
    4: (4, {"linemate": 1, "deraumere": 1, "sibur": 2, "phiras": 1}),
    5: (4, {"linemate": 1, "deraumere": 2, "sibur": 1, "mendiane": 3}),
    6: (6, {"linemate": 1, "deraumere": 2, "sibur": 3, "phiras": 1}),
    7: (6, {"linemate": 2, "deraumere": 2, "sibur": 2, "mendiane": 2, "phiras": 2, "thystame": 1}),
}

def parse_inv_payload(payload):
    result = {}
    for part in payload.split(","):
        if "=" in part:
            key, val = part.split("=")
            result[key] = int(val)
    return result

STONES = ["linemate", "deraumere", "sibur", "mendiane", "phiras", "thystame"]

def make_inv_payload(state):
    inv = state["stuff"]
    return ",".join(f"{s}={inv.get(s, 0)}" for s in STONES)

def broadcast_inventory(state):
    return f"Broadcast {Team_Password}:{state['id']}:{state['level']}:inv:{make_inv_payload(state)}"


def stone_total(state, ids):
    total = {s: state["stuff"].get(s, 0) for s in STONES}
    for i in ids:
        if i in state["team"] and state["team"][i]["type"] == "inv":
            other = parse_inv_payload(state["team"][i]["payload"])
            for key in total:
                total[key] += other.get(key, 0)
    return total

def team_thystame(state):
    total = state["stuff"].get("thystame", 0)
    for info in state["team"].values():
        if info["type"] == "inv":
            total += parse_inv_payload(info["payload"]).get("thystame", 0)
    return total


def has_recipe(total, level):
    for stone, qty in RECIPES[level][1].items():
        if total.get(stone, 0) < qty:
            return False
    return True
