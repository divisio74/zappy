#include "Hud.hpp"

#include <algorithm>
#include <iostream>
#include <map>
#include <utility>

namespace {
    const sf::Color HUD_PANEL(38, 18, 12, 220);
    const sf::Color HUD_PANEL_OUTLINE(166, 73, 28, 210);
    const sf::Color HUD_TITLE(255, 143, 44);
    const sf::Color HUD_ACCENT(255, 194, 95);
    const sf::Color HUD_TEXT(255, 238, 211);
    const sf::Color HUD_MUTED(211, 146, 101);

    void stylePanel(sf::RectangleShape &panel)
    {
        panel.setFillColor(HUD_PANEL);
        panel.setOutlineColor(HUD_PANEL_OUTLINE);
        panel.setOutlineThickness(2.f);
    }

    bool sortTeamByLevelDesc(const std::pair<std::string, int> &a, const std::pair<std::string, int> &b)
    {
        return a.second > b.second;
    }
}

void Hud::init()
{
    if (!_font.openFromFile("gui/fonts/arial.ttf"))
        std::cerr << "Failed to load HUD font: gui/fonts/arial.ttf" << std::endl;
    else
        _hasFont = true;

    if (!_cursorTexture.loadFromFile("gui/textures/cursor.png"))
        std::cerr << "Failed to load cursor: gui/textures/cursor.png" << std::endl;
    else
        _hasCursorTexture = true;
}

void Hud::release()
{
    _font = sf::Font();
    _cursorTexture = sf::Texture();
    _hasFont = false;
    _hasCursorTexture = false;
}

void Hud::render(sf::RenderWindow &window, const GameState &state, const std::optional<sf::Vector2i> &selectedTile)
{
    glUseProgram(0);
    glBindVertexArray(0);
    glDisable(GL_DEPTH_TEST);
    window.pushGLStates();
    window.resetGLStates();

    const float windowWidth = static_cast<float>(window.getSize().x);
    const float windowHeight = static_cast<float>(window.getSize().y);
    const float boardHeight = displayLeaderboard(window, state);

    displayTile(window, state, selectedTile, 5.f + boardHeight + 10.f);
    displayBroadcasts(window, state, windowWidth, windowHeight);
    displayCursor(window, windowWidth, windowHeight);

    window.popGLStates();
    glEnable(GL_DEPTH_TEST);
}

float Hud::displayLeaderboard(sf::RenderWindow &window, const GameState &state)
{
    std::map<std::string, int> teamLevel;

    for (const auto &entry : state.getPlayers()) {
        const Player &player = entry.second;

        if (teamLevel[player._teamName] < player._level)
            teamLevel[player._teamName] = player._level;
    }

    std::vector<std::pair<std::string, int>> teams(teamLevel.begin(), teamLevel.end());
    std::sort(teams.begin(), teams.end(), sortTeamByLevelDesc);

    int nbTeams = static_cast<int>(teams.size());
    if (nbTeams > 5)
        nbTeams = 5;

    const float height = 44.f + nbTeams * 27.f + 12.f;
    sf::RectangleShape leaderboardBg(sf::Vector2f(285.f, height));

    leaderboardBg.setPosition({5.f, 5.f});
    stylePanel(leaderboardBg);
    window.draw(leaderboardBg);

    drawLine(window, "=== Top equipes ===", 21, HUD_TITLE, {14.f, 10.f});
    for (int i = 0; i < nbTeams; i++) {
        const std::string line = std::to_string(i + 1) + ". " + teams[i].first + " - Lv " + std::to_string(teams[i].second);
        drawLine(window, line, 17, HUD_TEXT, {14.f, 42.f + i * 27.f});
    }
    return height;
}

void Hud::displayTile(sf::RenderWindow &window, const GameState &state, const std::optional<sf::Vector2i> &selectedTile, float top)
{
    if (!isTileValid(state, selectedTile))
        return;

    const int tileX = selectedTile->x;
    const int tileZ = selectedTile->y;
    const Tile &tile = state.getTile(tileX, tileZ);
    std::vector<const Player *> players;
    std::vector<const Egg *> eggs;

    tilePlayers(state, tileX, tileZ, players);
    tileEggs(state, tileX, tileZ, eggs);

    const int totalResources = tile.food + tile.linemate + tile.deraumere + tile.sibur
        + tile.mendiane + tile.phiras + tile.thystame;
    const std::vector<std::pair<std::string, int>> resources = tileResources(tile);
    const bool showAll = isExpanded();
    const std::size_t shownResources = limitCount(resources.size(), showAll);
    const std::size_t shownPlayers = limitCount(players.size(), showAll);
    const std::size_t shownEggs = limitCount(eggs.size(), showAll);
    const float height = panelHeight(
        shownResources,
        shownPlayers,
        shownEggs,
        resources.size() > shownResources,
        players.size() > shownPlayers,
        eggs.size() > shownEggs
    );

    sf::RectangleShape bg(sf::Vector2f(360.f, height));
    bg.setPosition({5.f, top});
    stylePanel(bg);
    window.draw(bg);

    float y = top + 12.f;
    drawLine(window, "Tile (" + std::to_string(tileX) + ", " + std::to_string(tileZ) + ")", 21, HUD_TITLE, 15.f, y);
    drawLine(window, "Resources total: " + std::to_string(totalResources), 17, HUD_TEXT, 15.f, y);
    displayResources(window, resources, shownResources, 15.f, y);

    drawLine(window, "Players: " + std::to_string(players.size()) + " | Eggs: " + std::to_string(eggs.size()), 17, HUD_ACCENT, 15.f, y);
    displayPlayers(window, players, shownPlayers, 15.f, y);
    displayEggs(window, eggs, shownEggs, 15.f, y);
}

void Hud::tilePlayers(const GameState &state, int tileX, int tileZ, std::vector<const Player *> &players) const
{
    for (const auto &entry : state.getPlayers()) {
        const Player &player = entry.second;

        if (player._x == tileX && player._y == tileZ)
            players.push_back(&player);
    }
}

void Hud::tileEggs(const GameState &state, int tileX, int tileZ, std::vector<const Egg *> &eggs) const
{
    for (const Egg &egg : state.getEggs()) {
        if (egg._x == tileX && egg._y == tileZ)
            eggs.push_back(&egg);
    }
}

std::vector<std::pair<std::string, int>> Hud::tileResources(const Tile &tile) const
{
    std::vector<std::pair<std::string, int>> resources;

    addResource(resources, "Food", tile.food);
    addResource(resources, "Linemate", tile.linemate);
    addResource(resources, "Deraumere", tile.deraumere);
    addResource(resources, "Sibur", tile.sibur);
    addResource(resources, "Mendiane", tile.mendiane);
    addResource(resources, "Phiras", tile.phiras);
    addResource(resources, "Thystame", tile.thystame);
    return resources;
}

void Hud::addResource(std::vector<std::pair<std::string, int>> &resources, const std::string &name, int count) const
{
    if (count > 0)
        resources.push_back({name, count});
}

std::size_t Hud::limitCount(std::size_t count, bool showAll) const
{
    if (showAll)
        return count;
    return std::min<std::size_t>(count, 4);
}

float Hud::panelHeight(std::size_t resourceLines, std::size_t playerLines, std::size_t eggLines,
    bool moreResources, bool morePlayers, bool moreEggs) const
{
    return 112.f
        + static_cast<float>(resourceLines) * 23.f
        + static_cast<float>(playerLines + eggLines) * 22.f
        + (moreResources ? 22.f : 0.f)
        + (morePlayers ? 22.f : 0.f)
        + (moreEggs ? 22.f : 0.f);
}

void Hud::displayResources(sf::RenderWindow &window, const std::vector<std::pair<std::string, int>> &resources,
    std::size_t shown, float x, float &y)
{
    if (resources.empty()) {
        drawLine(window, "No resources", 16, HUD_MUTED, x, y);
        return;
    }
    for (std::size_t i = 0; i < shown; i++)
        drawLine(window, resources[i].first + ": " + std::to_string(resources[i].second), 16, HUD_TEXT, x, y);
    if (resources.size() > shown)
        drawLine(window, "+" + std::to_string(resources.size() - shown) + " more resources", 15, HUD_MUTED, x, y);
}

void Hud::displayPlayers(sf::RenderWindow &window, const std::vector<const Player *> &players,
    std::size_t shown, float x, float &y)
{
    for (std::size_t i = 0; i < shown; i++) {
        const Player &player = *players[i];
        drawLine(window, "#" + std::to_string(player._id) + " " + player._teamName
            + " | Lvl :" + std::to_string(player._level)
            + " | Orientation : " + std::to_string(player._orientation)
            + " | Food : " + std::to_string(player._food), 15, HUD_TEXT, x, y);
    }
    if (players.size() > shown)
        drawLine(window, "+" + std::to_string(players.size() - shown) + " more players", 15, HUD_MUTED, x, y);
}

void Hud::displayEggs(sf::RenderWindow &window, const std::vector<const Egg *> &eggs,
    std::size_t shown, float x, float &y)
{
    for (std::size_t i = 0; i < shown; i++) {
        const Egg &egg = *eggs[i];
        drawLine(window, "Egg #" + std::to_string(egg._id) + " from player #" + std::to_string(egg._playerId), 15, HUD_TEXT, x, y);
    }
    if (eggs.size() > shown)
        drawLine(window, "+" + std::to_string(eggs.size() - shown) + " more eggs", 15, HUD_MUTED, x, y);
}

void Hud::displayBroadcasts(sf::RenderWindow &window, const GameState &state, float windowWidth, float windowHeight)
{
    const std::vector<std::pair<std::string, std::string>> &broadcasts = state.getBroadcasts();
    const float broadcastHeight = std::max(38.f, static_cast<float>(broadcasts.size()) * 26.f + 14.f);

    sf::RectangleShape broadcastBg(sf::Vector2f(520.f, broadcastHeight));
    broadcastBg.setPosition({windowWidth - 530.f, windowHeight - 20.f - broadcastHeight});
    stylePanel(broadcastBg);
    window.draw(broadcastBg);

    for (size_t i = 0; i < broadcasts.size(); i++) {
        const std::string line = "[" + broadcasts[i].first + "] " + broadcasts[i].second;
        drawLine(window, line, 17, HUD_TEXT, {
            windowWidth - 518.f,
            windowHeight - broadcastHeight - 12.f + static_cast<float>(i) * 26.f
        });
    }
}

void Hud::displayCursor(sf::RenderWindow &window, float windowWidth, float windowHeight)
{
    if (!_hasCursorTexture)
        return;

    sf::Sprite cursor(_cursorTexture);
    const sf::Vector2u size = _cursorTexture.getSize();

    cursor.setOrigin({static_cast<float>(size.x) / 2.f, static_cast<float>(size.y) / 2.f});
    cursor.setPosition({windowWidth / 2.f, windowHeight / 2.f});
    window.draw(cursor);
}

void Hud::drawLine(sf::RenderWindow &window, const std::string &line, unsigned int size, sf::Color color, sf::Vector2f position)
{
    if (!_hasFont)
        return;

    sf::Text text(_font, line, size);
    text.setFillColor(color);
    text.setPosition(position);
    window.draw(text);
}

void Hud::drawLine(sf::RenderWindow &window, const std::string &line, unsigned int size, sf::Color color, float x, float &y)
{
    drawLine(window, line, size, color, {x, y});
    y += static_cast<float>(size) + 7.f;
}

bool Hud::isExpanded() const
{
    return sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Add)
        || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Equal);
}

bool Hud::isTileValid(const GameState &state, const std::optional<sf::Vector2i> &selectedTile) const
{
    if (!selectedTile)
        return false;
    return selectedTile->x >= 0 && selectedTile->x < state.getWidth()
        && selectedTile->y >= 0 && selectedTile->y < state.getHeight();
}
