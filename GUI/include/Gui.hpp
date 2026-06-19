#pragma once

#include "Renderer.hpp"

class Gui {
    public:
        Gui();
        ~Gui() = default;

        void run();
        Renderer&        getRenderer();
        GameState&       getState();
        NetworkProtocol& getNetwork();

    private:
        Renderer            _renderer;
        GameState           _state;
        NetworkProtocol     _network;
        sf::Clock           _clock;
};
