#!/usr/bin/env python3

import math
import random

DIRS = [(0, 1), (1, 0), (0, -1), (-1, 0)]


def init_nav(state):
    state.setdefault("pos", (0, 0))
    state.setdefault("facing", 0)
    state.setdefault("visited", {})
    state.setdefault("nav_tick", 0)


def _cell(state):
    x, y = state["pos"]
    w = state.get("width") or 1
    h = state.get("height") or 1
    return (x % w, y % h)


def update_nav(state, command):
    init_nav(state)
    state["nav_tick"] += 1
    if command == "Forward":
        dx, dy = DIRS[state["facing"]]
        x, y = state["pos"]
        state["pos"] = (x + dx, y + dy)
    elif command == "Left":
        state["facing"] = (state["facing"] - 1) % 4
    elif command == "Right":
        state["facing"] = (state["facing"] + 1) % 4
    state["visited"][_cell(state)] = state["nav_tick"]


def index_to_rc(i):
    y = math.isqrt(i)
    return y, i - y * (y + 1)


def _has(vision, i, names):
    return i < len(vision) and any(n in vision[i] for n in names)


def forage(state, names):
    init_nav(state)
    vision = state["vision"]
    just_turned = state.get("last") in ("Left", "Right")
    if _has(vision, 2, names):
        return "Forward"
    if _has(vision, 1, names) and not just_turned:
        return "Left"
    if _has(vision, 3, names) and not just_turned:
        return "Right"
    for i in range(4, len(vision)):
        y, x = index_to_rc(i)
        if x == 0 and any(n in vision[i] for n in names):
            return "Forward"
    return explore(state)


def explore(state):
    init_nav(state)
    w = state.get("width") or 1
    h = state.get("height") or 1
    visited = state["visited"]
    x, y = state["pos"]
    options = []
    for cmd, face in (("Forward", state["facing"]),
                      ("Left", (state["facing"] - 1) % 4),
                      ("Right", (state["facing"] + 1) % 4)):
        dx, dy = DIRS[face]
        cell = ((x + dx) % w, (y + dy) % h)
        last_seen = visited.get(cell, -1)
        options.append((last_seen, random.random(), cmd))
    options.sort()
    return options[0][2]
