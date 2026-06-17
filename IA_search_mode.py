#!/usr/bin/env python3

import math
import random

from IA_helpers_meeting import partner_id, run_meeting_for2
from IA_helpers_together_inv import team_thystame, stone_total, has_recipe


def search_food(state):
    vision = state["vision"]
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
    return random.choice(["Forward", "Forward", "Left", "Right"])



def search_food_or_t(state):
    vision = state["vision"]
    total_thyst = team_thystame(state) == 0          # on chasse le thystame seulement si l'équipe n'en a AUCUN

    if total_thyst and "thystame" in vision[0]:
        return "Take thystame"
    if "food" in vision[0]:
        return "Take food"
    if state["last"] == "Right":
        return "Forward"
    if state["last"] == "Left":
        return "Forward"
    for i in range(len(vision)):
        y = int(math.sqrt(i))
        xx = y * (y + 1)
        x = i - xx
        if total_thyst and "thystame" in vision[xx]:
            return "Forward"
        if total_thyst and x < 0 and "thystame" in vision[i]:
            return "Left"
        if total_thyst and x > 0 and "thystame" in vision[i]:
            return "Right"
        if "food" in vision[xx]:
            return "Forward"
        if x < 0 and "food" in vision[i]:
            return "Left"
        if x > 0 and "food" in vision[i]:
            return "Right"
    return random.choice(["Forward", "Forward", "Left", "Right"])


def search_level1(state):
    vision = state["vision"]
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
    if state["child"] == False and len(state["team"]) < 6:
        state["child"] = True
        return "Fork"
    p_id = partner_id(state["id"])
    inv = stone_total(state, [p_id])
    comp = has_recipe(inv, 2)
    if comp == True:
        if vision[0].count("player") >= 2:
            if vision[0].count("sibur") < 1 and state["stuff"].get("sibur", 0) > 0:
                return "Set sibur"
            if vision[0].count("deraumere") < 1 and state["stuff"].get("deraumere", 0) > 0:
                return "Set deraumere"
            if vision[0].count("linemate") < 1 and state["stuff"].get("linemate", 0) > 0:
                return "Set linemate"
            if vision[0].count("linemate") >= 1 and vision[0].count("deraumere") >= 1 and vision[0].count("sibur") >= 1:
                if state["id"] < p_id:
                    return "Incantation"
            return "Look"
        verdicte = run_meeting_for2(state)
        if verdicte == "Unknow k":
            return "Look"
        return verdicte
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
        if "sibur" in vision[xx] or "deraumere" in vision[xx] or "linemate" in vision[xx]:
            return "Forward"
        if x < 0 and ("sibur" in vision[i] or "deraumere" in vision[i] or "linemate" in vision[i]):
            return "Left"
        if x > 0 and ("sibur" in vision[i] or "deraumere" in vision[i] or "linemate" in vision[i]):
            return "Right"
    return random.choice(["Forward", "Forward", "Left", "Right"])


def search_level3(state):
    vision = state["vision"]
    p_id = partner_id(state["id"])
    inv = stone_total(state, [p_id])
    comp = has_recipe(inv, 3)
    if comp == True:
        if vision[0].count("player") >= 2:
            if vision[0].count("sibur") < 1 and state["stuff"].get("sibur", 0) > 0:
                return "Set sibur"
            if vision[0].count("phiras") < 2 and state["stuff"].get("phiras", 0) > 0:
                return "Set phiras"
            if vision[0].count("linemate") < 2 and state["stuff"].get("linemate", 0) > 0:
                return "Set linemate"
            if vision[0].count("linemate") >= 2 and vision[0].count("phiras") >= 2 and vision[0].count("sibur") >= 1:
                if state["id"] < p_id:
                    return "Incantation"
            return "Look"
        verdicte = run_meeting_for2(state)
        if verdicte == "Unknow k":
            return "Look"
        return verdicte
    if "phiras" in vision[0]:
        return "Take phiras"
    if "sibur" in vision[0]:
        return "Take sibur"
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
        if "phiras" in vision[xx] or "sibur" in vision[xx] or "linemate" in vision[xx]:
            return "Forward"
        if x < 0 and ("phiras" in vision[i] or "sibur" in vision[i] or "linemate" in vision[i]):
            return "Left"
        if x > 0 and ("phiras" in vision[i] or "sibur" in vision[i] or "linemate" in vision[i]):
            return "Right"
    return random.choice(["Forward", "Forward", "Left", "Right"])

def search_level4_b6(state):
    vision = state["vision"]
    if "phiras" in vision[0]:
        return "Take phiras"
    if "mandiane" in vision[0]:
        return "Take mandiane"
    if "sibur" in vision[0]:
        return "Take sibur"
    if state["last"] == "Right":
        return "Forward"
    if state["last"] == "Left":
        return "Forward"
    for i in range(len(vision)):
        y = int(math.sqrt(i))
        xx = y * (y + 1)
        x = i - xx
        if "phiras" in vision[xx] or "mandiane" in vision[xx] or "sibur" in vision[xx]:
            return "Forward"
        if x < 0 and ("phiras" in vision[i] or "mandiane" in vision[i] or "sibur" in vision[i]):
            return "Left"
        if x > 0 and ("phiras" in vision[i] or "mandiane" in vision[i] or "sibur" in vision[i]):
            return "Right"
    return random.choice(["Forward", "Forward", "Left", "Right"])





