#pragma once

#include <SFML/Graphics.hpp>

#include "Camera.hpp"
#include "GameState.hpp"

class IRenderer {
    public:
        virtual ~IRenderer() = default;

        virtual const sf::VideoMode&    getVideoMode() const = 0;
        virtual const sf::RenderWindow& getWindow() const = 0;
        virtual sf::RenderWindow&       getWindow() = 0;
        virtual Camera&                 getCamera() = 0;

        virtual void init() = 0;
        virtual void shutdown() = 0;
        virtual void render(GameState& state, float deltaTime) = 0;
        virtual void toggleGroundOutline() = 0;
        virtual void selectTileAt(GameState& state, sf::Vector2i mousePosition) = 0;
};
