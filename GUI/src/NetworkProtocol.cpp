#include "../include/NetworkProtocol.hpp"
#include <iostream>
#include <sstream>
 
void NetworkProtocol::poll(GameState &state)
{
    std::vector<std::string> lines = receive();
 
    for (const std::string &line : lines) {
        if (!line.empty())
            parseMessage(line, state);
    }
}


void NetworkProtocol::parseMessage(const std::string &line, GameState &state)
{
    std::istringstream words(line);
    std::string cmd;
    words >> cmd;

    state._mutex.lock();

    if (cmd == "msz") {
        words >> state._width >> state._height;
        state._map.assign(state._height, std::vector<Tile>(state._width));
    }

    else if (cmd == "bct") {
        int x = 0;
        int y = 0;
        words >> x >> y;
        if (x >= 0 && x < state._width && y >= 0 && y < state._height) {
            Tile &t = state._map[y][x];
            words >> t.food >> t.linemate >> t.deraumere
                  >> t.sibur >> t.mendiane >> t.phiras >> t.thystame;
        }
    }

    else if (cmd == "tna") {
        std::string name;
        words >> name;
        state._teams.push_back(name);
    }

    else if (cmd == "pnw") {
    }

    else if (cmd == "ppo") {
    }

    else if (cmd == "plv") {
    }

    else if (cmd == "pin") {
    }

    else if (cmd == "pex") {
    }

    else if (cmd == "pbc") {
    }

    else if (cmd == "pic") {
    }

    else if (cmd == "pie") {
    }

    else if (cmd == "pfk") {
    }

    else if (cmd == "pdr") {
    }

    else if (cmd == "pgt") {
    }

    else if (cmd == "pdi") {
    }

    else if (cmd == "enw") {
    }

    else if (cmd == "ebo") {
    }

    else if (cmd == "edi") {
    }

    else if (cmd == "sgt") {
        words >> state._timeUnit;
    }

    else if (cmd == "sst") {
        words >> state._timeUnit;
    }

    else if (cmd == "seg") {
        words >> state._winner;
        state._gameOver = true;
    }

    else if (cmd == "smg") {
        std::string msg;
        std::getline(words, msg);
        std::cout << "[serveur] " << msg << "\n";
    }

    else if (cmd == "suc") {
        std::cerr << "NetworkHandler: commande inconnue rejetée\n";
    }

    else if (cmd == "sbp") {
        std::cerr << "NetworkHandler: paramètre invalide\n";
    }

    state._mutex.unlock();
}
