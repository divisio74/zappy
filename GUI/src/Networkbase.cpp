#include "../include/Networkbase.hpp"
#include <cerrno>
#include <iostream>
#include <sys/select.h>
 
NetworkBase::NetworkBase()
    : _connected(false)
{
}

bool NetworkBase::connect(const std::string &host, int port)
{
    try {
        _socket = std::make_unique<ClientSocket>(host, port);
    } catch (const std::runtime_error &e) {
        std::cerr << "NetworkBase: connexion échouée : " << e.what() << "\n";
        return false;
    }

    std::vector<std::string> lines = _socket->read();
    if (lines.empty() || lines[0] != "WELCOME") {
        std::cerr << "NetworkBase: réponse inattendue du serveur\n";
        return false;
    }
    _socket->write("GRAPHIC");
    _socket->write("msz");
    _socket->write("mct");
    _connected = true;
    std::cout << "NetworkBase: connecté au serveur Zappy\n";
    return true;
}

std::vector<std::string> NetworkBase::receive()
{
    if (!_connected)
        return {};

    fd_set readFds;
    timeval timeout{};
    int fd = _socket->getFd();

    FD_ZERO(&readFds);
    FD_SET(fd, &readFds);
    int ready = select(fd + 1, &readFds, nullptr, nullptr, &timeout);
    if (ready == 0)
        return {};
    if (ready == -1) {
        if (errno == EINTR)
            return {};
        std::cerr << "NetworkBase: select échoué\n";
        _connected = false;
        return {};
    }

    try {
        return _socket->read();
    } catch (const std::runtime_error &e) {
        std::cerr << "NetworkBase: connexion perdue : " << e.what() << "\n";
        _connected = false;
        return {};
    }
}

void NetworkBase::send(const std::string &cmd)
{
    if (!_connected)
        return;

    try {
        _socket->write(cmd);
    } catch (const std::runtime_error &e) {
        std::cerr << "NetworkBase: envoi échoué : " << e.what() << "\n";
        _connected = false;
    }
}
