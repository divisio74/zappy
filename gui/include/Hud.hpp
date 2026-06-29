#pragma once

#include <GL/glew.h>
#include <SFML/Graphics.hpp>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "GameState.hpp"

class Hud {
    public:
        void init();
        void release();
        void render(sf::RenderWindow &window, const GameState &state, const std::optional<sf::Vector2i> &selectedTile);

    private:
        float displayLeaderboard(sf::RenderWindow &window, const GameState &state);
        void displayTile(sf::RenderWindow &window, const GameState &state, const std::optional<sf::Vector2i> &selectedTile, float top);
        void tilePlayers(const GameState &state, int tileX, int tileZ, std::vector<const Player *> &players) const;
        void tileEggs(const GameState &state, int tileX, int tileZ, std::vector<const Egg *> &eggs) const;
        std::vector<std::pair<std::string, int>> tileResources(const Tile &tile) const;
        void addResource(std::vector<std::pair<std::string, int>> &resources, const std::string &name, int count) const;
        std::size_t limitCount(std::size_t count, bool showAll) const;
        float panelHeight(std::size_t resourceLines, std::size_t playerLines, std::size_t eggLines,
            bool moreResources, bool morePlayers, bool moreEggs) const;
        void displayResources(sf::RenderWindow &window, const std::vector<std::pair<std::string, int>> &resources,
            std::size_t shown, float x, float &y);
        void displayPlayers(sf::RenderWindow &window, const std::vector<const Player *> &players,
            std::size_t shown, float x, float &y);
        void displayEggs(sf::RenderWindow &window, const std::vector<const Egg *> &eggs,
            std::size_t shown, float x, float &y);
        void displayBroadcasts(sf::RenderWindow &window, const GameState &state, float windowWidth, float windowHeight);
        void displayCursor(sf::RenderWindow &window, float windowWidth, float windowHeight);
        void drawLine(sf::RenderWindow &window, const std::string &line, unsigned int size, sf::Color color, sf::Vector2f position);
        void drawLine(sf::RenderWindow &window, const std::string &line, unsigned int size, sf::Color color, float x, float &y);
        bool isExpanded() const;
        bool isTileValid(const GameState &state, const std::optional<sf::Vector2i> &selectedTile) const;

        sf::Font    _font;
        sf::Texture _cursorTexture;
        bool        _hasFont = false;
        bool        _hasCursorTexture = false;
};
