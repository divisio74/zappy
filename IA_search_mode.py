#!/usr/bin/env python3

import math
import random

from IA_helpers_meeting import run_meeting_for6
from IA_helpers_together_inv import team_thystame, stone_total, has_recipe, get_id, need_stone
from IA_navigation import forage


def search_food_or_t(state):
    vision = state["vision"]
    total_thyst = team_thystame(state) == 0

    if total_thyst and "thystame" in vision[0]:
        return "Take thystame"
    if "food" in vision[0]:
        return "Take food"
    targets = ["food", "thystame"] if total_thyst else ["food"]
    return forage(state, targets)


def search_level1(state):
    vision = state["vision"]
    if state["id"] == 1 and len(state["spawned_ids"]) < 5:
        return "Fork"
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
    return random.choice(["Forward", "Forward", "Left", "Right"])

def search_level2(state):
    vision = state["vision"]
    ids = get_id(state)
    inv = stone_total(state, ids)
    comp = has_recipe(inv, 2)
    if vision[0].count("player") >= 6:
        if vision[0].count("sibur") < 1 and state["stuff"].get("sibur", 0) > 0:
            return "Set sibur"
        if vision[0].count("deraumere") < 1 and state["stuff"].get("deraumere", 0) > 0:
            return "Set deraumere"
        if vision[0].count("linemate") < 1 and state["stuff"].get("linemate", 0) > 0:
            return "Set linemate"
        if vision[0].count("linemate") >= 1 and vision[0].count("deraumere") >= 1 and vision[0].count("sibur") >= 1:
            if not ids or state["id"] < min(ids):
                return "Incantation"
        return "Look"
    if comp == True:
        verdicte = run_meeting_for6(state)
        if verdicte == "Unknow k" or verdicte == "final_destination":
            return "Look"
        return verdicte
    if "sibur" in vision[0] and need_stone(state, ids, "sibur", 2):
        return "Take sibur"
    if "deraumere" in vision[0] and need_stone(state, ids, "deraumere", 2):
        return "Take deraumere"
    if "linemate" in vision[0] and need_stone(state, ids, "linemate", 2):
        return "Take linemate"
    if state["last"] == "Right":
        return "Forward"
    if state["last"] == "Left":
        return "Forward"
    for i in range(len(vision)):
        y = int(math.sqrt(i))
        xx = y * (y + 1)
        x = i - xx
        if "sibur" in vision[xx] or "deraumere" in vision[xx] or "linemate" in vision[xx]:
            return "Forward"
        if x < 0 and ("sibur" in vision[i] or "deraumere" in vision[i] or "linemate" in vision[i]):
            return "Left"
        if x > 0 and ("sibur" in vision[i] or "deraumere" in vision[i] or "linemate" in vision[i]):
            return "Right"
    return random.choice(["Forward", "Forward", "Left", "Right"])


def search_level3(state):
    vision = state["vision"]
    ids = get_id(state)
    inv = stone_total(state, ids)
    comp = has_recipe(inv, 3)
    if vision[0].count("player") >= 6:
        if vision[0].count("sibur") < 1 and state["stuff"].get("sibur", 0) > 0:
            return "Set sibur"
        if vision[0].count("phiras") < 2 and state["stuff"].get("phiras", 0) > 0:
            return "Set phiras"
        if vision[0].count("linemate") < 2 and state["stuff"].get("linemate", 0) > 0:
            return "Set linemate"
        if vision[0].count("linemate") >= 2 and vision[0].count("phiras") >= 2 and vision[0].count("sibur") >= 1:
            if not ids or state["id"] < min(ids):
                return "Incantation"
        return "Look"
    if comp == True:
        verdicte = run_meeting_for6(state)
        if verdicte == "final_destination":
            return "Look"
        return verdicte
    if "phiras" in vision[0] and need_stone(state, ids, "phiras", 3):
        return "Take phiras"
    if "sibur" in vision[0] and need_stone(state, ids, "sibur", 3):
        return "Take sibur"
    if "linemate" in vision[0] and need_stone(state, ids, "linemate", 3):
        return "Take linemate"
    if state["last"] == "Right":
        return "Forward"
    if state["last"] == "Left":
        return "Forward"
    for i in range(len(vision)):
        y = int(math.sqrt(i))
        xx = y * (y + 1)
        x = i - xx
        if "phiras" in vision[xx] or "sibur" in vision[xx] or "linemate" in vision[xx]:
            return "Forward"
        if x < 0 and ("phiras" in vision[i] or "sibur" in vision[i] or "linemate" in vision[i]):
            return "Left"
        if x > 0 and ("phiras" in vision[i] or "sibur" in vision[i] or "linemate" in vision[i]):
            return "Right"
    return random.choice(["Forward", "Forward", "Left", "Right"])


def search_level4(state):
    vision = state["vision"]
    ids = get_id(state)
    inv = stone_total(state, ids)
    comp = has_recipe(inv, 4)
    if vision[0].count("player") >= 6:
        if vision[0].count("phiras") < 1 and state["stuff"].get("phiras", 0) > 0:
            return "Set phiras"
        if vision[0].count("sibur") < 2 and state["stuff"].get("sibur", 0) > 0:
            return "Set sibur"
        if vision[0].count("deraumere") < 1 and state["stuff"].get("deraumere", 0) > 0:
            return "Set deraumere"
        if vision[0].count("linemate") < 1 and state["stuff"].get("linemate", 0) > 0:
            return "Set linemate"
        if vision[0].count("linemate") >= 1 and vision[0].count("phiras") >= 1 and vision[0].count("sibur") >= 2 and vision[0].count("deraumere") >= 1:
            if not ids or state["id"] < min(ids):
                return "Incantation"
        return "Look"
    if comp == True:
        verdicte = run_meeting_for6(state)
        if verdicte == "Unknow k" or verdicte == "final_destination":
            return "Look"
        return verdicte
    if "phiras" in vision[0] and need_stone(state, ids, "phiras", 4):
        return "Take phiras"
    if "sibur" in vision[0] and need_stone(state, ids, "sibur", 4):
        return "Take sibur"
    if "deraumere" in vision[0] and need_stone(state, ids, "deraumere", 4):
        return "Take deraumere"
    if "linemate" in vision[0] and need_stone(state, ids, "linemate", 4):
        return "Take linemate"
    if state["last"] == "Right":
        return "Forward"
    if state["last"] == "Left":
        return "Forward"
    for i in range(len(vision)):
        y = int(math.sqrt(i))
        xx = y * (y + 1)
        x = i - xx
        if "phiras" in vision[xx] or "sibur" in vision[xx] or "linemate" in vision[xx] or "deraumere" in vision[xx]:
            return "Forward"
        if x < 0 and ("phiras" in vision[i] or "sibur" in vision[i] or "linemate" in vision[i] or "deraumere" in vision[i]):
            return "Left"
        if x > 0 and ("phiras" in vision[i] or "sibur" in vision[i] or "linemate" in vision[i] or "deraumere" in vision[i]):
            return "Right"
    return random.choice(["Forward", "Forward", "Left", "Right"])


def search_level5(state):
    vision = state["vision"]
    ids = get_id(state)
    inv = stone_total(state, ids)
    comp = has_recipe(inv, 5)
    if vision[0].count("player") >= 6:
        if vision[0].count("mendiane") < 3 and state["stuff"].get("mendiane", 0) > 0:
            return "Set mendiane"
        if vision[0].count("deraumere") < 2 and state["stuff"].get("deraumere", 0) > 0:
            return "Set deraumere"
        if vision[0].count("sibur") < 1 and state["stuff"].get("sibur", 0) > 0:
            return "Set sibur"
        if vision[0].count("linemate") < 1 and state["stuff"].get("linemate", 0) > 0:
            return "Set linemate"
        if vision[0].count("linemate") >= 1 and vision[0].count("deraumere") >= 2 and vision[0].count("sibur") >= 1 and vision[0].count("mendiane") >= 3:
            if not ids or state["id"] < min(ids):
                return "Incantation"
        return "Look"
    if comp == True:
        verdicte = run_meeting_for6(state)
        if verdicte == "Unknow k" or verdicte == "final_destination":
            return "Look"
        return verdicte
    if "mendiane" in vision[0] and need_stone(state, ids, "mendiane", 5):
        return "Take mendiane"
    if "deraumere" in vision[0] and need_stone(state, ids, "deraumere", 5):
        return "Take deraumere"
    if "sibur" in vision[0] and need_stone(state, ids, "sibur", 5):
        return "Take sibur"
    if "linemate" in vision[0] and need_stone(state, ids, "linemate", 5):
        return "Take linemate"
    if state["last"] == "Right":
        return "Forward"
    if state["last"] == "Left":
        return "Forward"
    for i in range(len(vision)):
        y = int(math.sqrt(i))
        xx = y * (y + 1)
        x = i - xx
        if "mendiane" in vision[xx] or "deraumere" in vision[xx] or "sibur" in vision[xx] or "linemate" in vision[xx]:
            return "Forward"
        if x < 0 and ("mendiane" in vision[i] or "deraumere" in vision[i] or "sibur" in vision[i] or "linemate" in vision[i]):
            return "Left"
        if x > 0 and ("mendiane" in vision[i] or "deraumere" in vision[i] or "sibur" in vision[i] or "linemate" in vision[i]):
            return "Right"
    return random.choice(["Forward", "Forward", "Left", "Right"])


def search_level6(state):
    vision = state["vision"]
    ids = get_id(state)
    inv = stone_total(state, ids)
    comp = has_recipe(inv, 6)
    if vision[0].count("player") >= 6:
        if vision[0].count("sibur") < 3 and state["stuff"].get("sibur", 0) > 0:
            return "Set sibur"
        if vision[0].count("deraumere") < 2 and state["stuff"].get("deraumere", 0) > 0:
            return "Set deraumere"
        if vision[0].count("phiras") < 1 and state["stuff"].get("phiras", 0) > 0:
            return "Set phiras"
        if vision[0].count("linemate") < 1 and state["stuff"].get("linemate", 0) > 0:
            return "Set linemate"
        if vision[0].count("linemate") >= 1 and vision[0].count("deraumere") >= 2 and vision[0].count("sibur") >= 3 and vision[0].count("phiras") >= 1:
            if not ids or state["id"] < min(ids):
                return "Incantation"
        return "Look"
    if comp == True:
        verdicte = run_meeting_for6(state)
        if verdicte == "Unknow k" or verdicte == "final_destination":
            return "Look"
        return verdicte
    if "sibur" in vision[0] and need_stone(state, ids, "sibur", 6):
        return "Take sibur"
    if "deraumere" in vision[0] and need_stone(state, ids, "deraumere", 6):
        return "Take deraumere"
    if "phiras" in vision[0] and need_stone(state, ids, "phiras", 6):
        return "Take phiras"
    if "linemate" in vision[0] and need_stone(state, ids, "linemate", 6):
        return "Take linemate"
    if state["last"] == "Right":
        return "Forward"
    if state["last"] == "Left":
        return "Forward"
    for i in range(len(vision)):
        y = int(math.sqrt(i))
        xx = y * (y + 1)
        x = i - xx
        if "sibur" in vision[xx] or "deraumere" in vision[xx] or "phiras" in vision[xx] or "linemate" in vision[xx]:
            return "Forward"
        if x < 0 and ("sibur" in vision[i] or "deraumere" in vision[i] or "phiras" in vision[i] or "linemate" in vision[i]):
            return "Left"
        if x > 0 and ("sibur" in vision[i] or "deraumere" in vision[i] or "phiras" in vision[i] or "linemate" in vision[i]):
            return "Right"
    return random.choice(["Forward", "Forward", "Left", "Right"])


def search_level7(state):
    vision = state["vision"]
    ids = get_id(state)
    inv = stone_total(state, ids)
    comp = has_recipe(inv, 7)
    if vision[0].count("player") >= 6:
        if vision[0].count("thystame") < 1 and state["stuff"].get("thystame", 0) > 0:
            return "Set thystame"
        if vision[0].count("mendiane") < 2 and state["stuff"].get("mendiane", 0) > 0:
            return "Set mendiane"
        if vision[0].count("phiras") < 2 and state["stuff"].get("phiras", 0) > 0:
            return "Set phiras"
        if vision[0].count("sibur") < 2 and state["stuff"].get("sibur", 0) > 0:
            return "Set sibur"
        if vision[0].count("deraumere") < 2 and state["stuff"].get("deraumere", 0) > 0:
            return "Set deraumere"
        if vision[0].count("linemate") < 2 and state["stuff"].get("linemate", 0) > 0:
            return "Set linemate"
        if vision[0].count("linemate") >= 2 and vision[0].count("deraumere") >= 2 and vision[0].count("sibur") >= 2 and vision[0].count("mendiane") >= 2 and vision[0].count("phiras") >= 2 and vision[0].count("thystame") >= 1:
            if not ids or state["id"] < min(ids):
                return "Incantation"
        return "Look"
    if comp == True:
        verdicte = run_meeting_for6(state)
        if verdicte == "Unknow k" or verdicte == "final_destination":
            return "Look"
        return verdicte
    if "thystame" in vision[0] and need_stone(state, ids, "thystame", 7):
        return "Take thystame"
    if "mendiane" in vision[0] and need_stone(state, ids, "mendiane", 7):
        return "Take mendiane"
    if "phiras" in vision[0] and need_stone(state, ids, "phiras", 7):
        return "Take phiras"
    if "sibur" in vision[0] and need_stone(state, ids, "sibur", 7):
        return "Take sibur"
    if "deraumere" in vision[0] and need_stone(state, ids, "deraumere", 7):
        return "Take deraumere"
    if "linemate" in vision[0] and need_stone(state, ids, "linemate", 7):
        return "Take linemate"
    if state["last"] == "Right":
        return "Forward"
    if state["last"] == "Left":
        return "Forward"
    for i in range(len(vision)):
        y = int(math.sqrt(i))
        xx = y * (y + 1)
        x = i - xx
        if "thystame" in vision[xx] or "mendiane" in vision[xx] or "phiras" in vision[xx] or "sibur" in vision[xx] or "deraumere" in vision[xx] or "linemate" in vision[xx]:
            return "Forward"
        if x < 0 and ("thystame" in vision[i] or "mendiane" in vision[i] or "phiras" in vision[i] or "sibur" in vision[i] or "deraumere" in vision[i] or "linemate" in vision[i]):
            return "Left"
        if x > 0 and ("thystame" in vision[i] or "mendiane" in vision[i] or "phiras" in vision[i] or "sibur" in vision[i] or "deraumere" in vision[i] or "linemate" in vision[i]):
            return "Right"
    return random.choice(["Forward", "Forward", "Left", "Right"])
