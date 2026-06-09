#pragma once
#include <string>
#include <memory>
#include "network/ClientSocket.hpp"
#include "GameState.hpp"

class NetworkHandler {
    public:
        NetworkHandler();
        ~NetworkHandler() = default;

        bool connect(const std::string &host, int port);
        void poll(GameState &state);
        void send(const std::string &cmd);
        void flush();

        bool isConnected() const {return _connected;}

    private:
        std::unique_ptr<ClientSocket> _socket;
        bool _connected;
        std::string _sendBuffer;

        void parseMessage(const std::string &line, GameState &state);
};
 

