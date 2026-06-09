
#include "../include/networkHandler.hpp"
#include <iostream>
#include <sstream>

NetworkHandler::NetworkHandler()
    : _connected(false)
{
}

bool NetworkHandler::connect(const std::string &host, int port)
{
    try {
        _socket = std::make_unique<ClientSocket>(host, (uint16_t)port);
    } catch (const std::runtime_error &e) {
        std::cerr << "NetworkHandler: connexion échouée : " << e.what() << "\n";
        return false;
    }

    std::vector<std::string> lines = _socket->read();
    if (lines.empty() || lines[0] != "WELCOME") {
        std::cerr << "NetworkHandler: réponse inattendue du serveur\n";
        return false;
    }
    _socket->write("GRAPHIC");
    _connected = true;
    std::cout << "NetworkHandler: connecté au serveur Zappy en bien bien\n";
    return true;
}


void NetworkHandler::poll(GameState &state)
{
    if (!_connected)
        return;
 
    std::vector<std::string> lines;

    try {
        lines = _socket->read();
    } catch (const std::runtime_error &e) {
        std::cerr << "NetworkHandler: connexion perdue : " << e.what() << "\n";
        _connected = false;
        return;
    }

    for (const std::string &line : lines) {
        if (!line.empty())
            parseMessage(line, state);
    }
}

void NetworkHandler::parseMessage(const std::string &line, GameState &state)
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

void NetworkHandler::send(const std::string &cmd)
{
    _sendBuffer += cmd;
}

void NetworkHandler::flush()
{
    if (!_connected || _sendBuffer.empty())
        return;

    try {
        _socket->write(_sendBuffer);
        _sendBuffer.clear();
    } catch (const std::runtime_error &e) {
        std::cerr << "NetworkHandler: envoi échoué : " << e.what() << "\n";
        _connected = false;
    }
}
