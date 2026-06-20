#!/usr/bin/env python3

import math
import random

from IA_helpers_net import Team_Password

def find_direction(k, state):
    if k == 0:
        return "final_destination"
    if k == 1 or k == 2 or k == 8:
        return "Forward"
    if (k == 3 or k == 4 or k == 5) and state["last"] != "Right":
        return "Left"
    if (k == 6 or k == 7) and state["last"] != "Left":
        return "Right"
    return random.choice(["Forward", "Left", "Right"])



def homing_vision(state):
    vision = state["vision"]
    best = -1
    best_count = 2
    for i in range(1, len(vision)):
        if vision[i].count("player") > best_count:
            best_count = vision[i].count("player")
            best = i
    if best < 0:
        return None
    y = int(math.sqrt(best))
    xx = y * (y + 1)
    x = best - xx
    if x == 0 or y == 1:
        return "Forward"
    if x < 0 and state["last"] == "Right":
        return random.choice(["Forward", "Left", "Right"])
    if x < 0 and state["last"] != "Right":
        return "Left"
    return "Right"


def run_meeting_for6(state):
    if not state["team"]:
        return f"Broadcast {Team_Password}:{state['id']}:{state['level']}:rally"
    balise_id = min(state["team"])
    if balise_id < state["id"]:
        v = homing_vision(state)
        if v is not None:
            return v
        info = state["team"][balise_id]
        if info.get("seq") == state.get("acted_seq"):
            return "Inventory"
        state["acted_seq"] = info.get("seq")
        return find_direction(info["k"], state)
    return f"Broadcast {Team_Password}:{state['id']}:{state['level']}:rally"

def check_phase2(state):
    for info in state["team"].values():
        if info["level"] < 4:
            return False
    if len(state["team"]) != 5:
        return False
    return True