#!/usr/bin/env python3

from IA_helpers_net import Team_Password

def parsing_vision(line):
    inner = line.strip().lstrip("[").rstrip("]")
    return [cell.split() for cell in inner.split(",")]


def parsing_stuff(line):
    inner = line.strip().lstrip("[").rstrip("]")
    inv = {}
    for item in inner.split(","):
        parts = item.split()
        if len(parts) == 2 and parts[1].isdigit():
            inv[parts[0]] = int(parts[1])
    return inv


def get_response(conn, state):
    while True:
        line = conn.readline()
        if line.startswith("message"):
            handle_broadcast(line, state)
            continue
        if line == "Elevation underway":
            continue
        if line.startswith("Current level"):
            state["level"] = int(line.split()[-1])
            continue
        return line


def read_incantation_result(conn, state):
    while True:
        line = conn.readline()
        if line.startswith("message"):
            handle_broadcast(line, state)
            continue
        if line == "Elevation underway":
            continue
        if line.startswith("Current level"):
            state["level"] = int(line.split()[-1])
            return line
        return line


def handle_broadcast(line, state):
    try:
        body = line[len("message "):]
        k_str, text = body.split(", ", 1)
        k = int(k_str)
        parts = text.split(":")
        if len(parts) < 4 or parts[0] != Team_Password:
            return
        sender_id = int(parts[1])
        if sender_id == state["id"]:
            return
        level = int(parts[2])
        msg_type = parts[3]
        payload = parts[4] if len(parts) > 4 else ""

        state["team"][sender_id] = {
            "k": k,
            "level": level,
            "type": msg_type,
            "payload": payload,
        }

    except (ValueError, IndexError):
        return