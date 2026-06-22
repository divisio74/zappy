#!/usr/bin/env python3

import socket

Team_Password = "_yaR_OFJv"

class Connection:
    def __init__(self, host, port):
        self.sock = socket.create_connection((host, port))
        self.buffer = ""

    def readline(self):
        while "\n" not in self.buffer:
            chunk = self.sock.recv(4096)
            if not chunk:
                raise ConnectionError("serveur deconnecté")
            self.buffer += chunk.decode(errors="replace")
        line, self.buffer = self.buffer.split("\n", 1)
        return line

    def send(self, command):
        self.sock.sendall((command + "\n").encode())

    def close(self):
        self.sock.close()


def handshake(conn, team_name):
    welcome = conn.readline()
    if welcome != "WELCOME":
        raise ConnectionError(f"attendu 'WELCOME', recu {welcome!r}")

    conn.send(team_name)

    slots = int(conn.readline())
    width, height = map(int, conn.readline().split())
    return slots, width, height
