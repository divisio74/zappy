#!/usr/bin/env python3

from IA_helpers_net import Team_Password

def guess_direction(k):
    if k == 0:
        return "final_destination"
    if k == 1 or k == 2 or k == 8:
        return "Forward"
    if k == 3 or k == 4 or k == 5:
        return "Left"
    if k == 6 or k == 7:
        return "Right"
    if k > 8 or k < 0:
        return "Unknow k"

def broadcast_meeting_detection(state):
    for sender_id in state["team"]:
        if sender_id < state["id"]:
            return "follower"
    return "balise"

def partner_id(my_id):
    return my_id + 1 if my_id % 2 == 1 else my_id - 1

def run_meeting_for2(state):
    p = partner_id(state["id"])
    if state["id"] < p:
        return f"Broadcast {Team_Password}:{state['id']}:rally"
    if p in state["team"]:
        return guess_direction(state["team"][p]["k"])
    return "Look"


def run_meeting_for6(state):
    if not state["team"]:
        return f"Broadcast {Team_Password}:{state['id']}:rally"
    balise_id = min(state["team"])
    if balise_id < state["id"]:
        return guess_direction(state["team"][balise_id]["k"])
    return f"Broadcast {Team_Password}:{state['id']}:rally"