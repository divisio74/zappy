#pragma once

#include <SFML/Graphics.hpp>

class App {
    public:
        App();
        ~App() = default;

        void run();

    private:
        sf::VideoMode _videoMode;
        sf::RenderWindow _renderWindow;
};