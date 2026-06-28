#pragma once

#include <GL/glew.h>
#include <SFML/Graphics.hpp>
#include <optional>
#include <string>
#include <vector>

#include "GameState.hpp"

class Hud {
    public:
        void init();
        void release();
        void render(sf::RenderWindow &window, const GameState &state, const std::optional<sf::Vector2i> &selectedTile);

    private:
        float renderLeaderboard(sf::RenderWindow &window, const GameState &state);
        void renderSelectedTile(sf::RenderWindow &window, const GameState &state, const std::optional<sf::Vector2i> &selectedTile, float top);
        void renderBroadcasts(sf::RenderWindow &window, const GameState &state, float windowWidth, float windowHeight);
        void renderCursor(sf::RenderWindow &window, float windowWidth, float windowHeight);
        void drawLine(sf::RenderWindow &window, const std::string &line, unsigned int size, sf::Color color, sf::Vector2f position);
        void drawLine(sf::RenderWindow &window, const std::string &line, unsigned int size, sf::Color color, float x, float &y);
        bool isTileValid(const GameState &state, const std::optional<sf::Vector2i> &selectedTile) const;

        sf::Font    _font;
        sf::Texture _cursorTexture;
        bool        _hasFont = false;
        bool        _hasCursorTexture = false;
};
