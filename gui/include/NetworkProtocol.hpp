#pragma once
#include "Networkbase.hpp"
#include "GameState.hpp"

class NetworkProtocol : public NetworkBase {
    public:
        NetworkProtocol() = default;
        ~NetworkProtocol() = default;
        void poll(GameState &state);
    private:
        void parseMessage(const std::string &line, GameState &state);
};
