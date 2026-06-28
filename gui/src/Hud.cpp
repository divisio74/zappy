#include "Hud.hpp"

#include <algorithm>
#include <iostream>
#include <map>

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
    const float leaderboardHeight = renderLeaderboard(window, state);

    renderSelectedTile(window, state, selectedTile, 5.f + leaderboardHeight + 10.f);
    renderBroadcasts(window, state, windowWidth, windowHeight);
    renderCursor(window, windowWidth, windowHeight);

    window.popGLStates();
    glEnable(GL_DEPTH_TEST);
}

float Hud::renderLeaderboard(sf::RenderWindow &window, const GameState &state)
{
    std::map<std::string, int> teamLevel;

    for (auto &[id, player] : state.getPlayers()) {
        if (teamLevel[player._teamName] < player._level)
            teamLevel[player._teamName] = player._level;
    }

    std::vector<std::pair<std::string, int>> teams(teamLevel.begin(), teamLevel.end());
    std::sort(teams.begin(), teams.end(), [](const std::pair<std::string, int> &a, const std::pair<std::string, int> &b) {
        return a.second > b.second;
    });

    int nbTeams = static_cast<int>(teams.size());
    if (nbTeams > 5)
        nbTeams = 5;

    const float leaderboardHeight = 44.f + nbTeams * 27.f + 12.f;
    sf::RectangleShape leaderboardBg(sf::Vector2f(285.f, leaderboardHeight));

    leaderboardBg.setPosition({5.f, 5.f});
    stylePanel(leaderboardBg);
    window.draw(leaderboardBg);

    drawLine(window, "=== Top equipes ===", 21, HUD_TITLE, {14.f, 10.f});
    for (int i = 0; i < nbTeams; i++) {
        const std::string line = std::to_string(i + 1) + ". " + teams[i].first + " - Lv " + std::to_string(teams[i].second);
        drawLine(window, line, 17, HUD_TEXT, {14.f, 42.f + i * 27.f});
    }
    return leaderboardHeight;
}

void Hud::renderSelectedTile(sf::RenderWindow &window, const GameState &state, const std::optional<sf::Vector2i> &selectedTile, float top)
{
    if (!isTileValid(state, selectedTile))
        return;

    const int tileX = selectedTile->x;
    const int tileZ = selectedTile->y;
    const Tile &tile = state.getTile(tileX, tileZ);
    std::vector<const Player *> players;
    std::vector<const Egg *> eggs;

    for (const auto &[id, player] : state.getPlayers()) {
        if (player._x == tileX && player._y == tileZ)
            players.push_back(&player);
    }
    for (const Egg &egg : state.getEggs()) {
        if (egg._x == tileX && egg._y == tileZ)
            eggs.push_back(&egg);
    }

    const int totalResources = tile.food + tile.linemate + tile.deraumere + tile.sibur
        + tile.mendiane + tile.phiras + tile.thystame;
    const std::size_t maxPlayersShown = std::min<std::size_t>(players.size(), 4);
    const float panelHeight = 246.f + static_cast<float>(maxPlayersShown + eggs.size()) * 22.f;

    sf::RectangleShape bg(sf::Vector2f(360.f, panelHeight));
    bg.setPosition({5.f, top});
    stylePanel(bg);
    window.draw(bg);

    float y = top + 12.f;
    drawLine(window, "Tile (" + std::to_string(tileX) + ", " + std::to_string(tileZ) + ")", 21, HUD_TITLE, 15.f, y);
    drawLine(window, "Resources total: " + std::to_string(totalResources), 17, HUD_TEXT, 15.f, y);
    drawLine(window, "Food: " + std::to_string(tile.food) + " | Linemate: " + std::to_string(tile.linemate), 16, HUD_TEXT, 15.f, y);
    drawLine(window, "Deraumere: " + std::to_string(tile.deraumere) + " | Sibur: " + std::to_string(tile.sibur), 16, HUD_TEXT, 15.f, y);
    drawLine(window, "Mendiane: " + std::to_string(tile.mendiane) + " | Phiras: " + std::to_string(tile.phiras), 16, HUD_TEXT, 15.f, y);
    drawLine(window, "Thystame: " + std::to_string(tile.thystame), 16, HUD_TEXT, 15.f, y);
    drawLine(window, "Players: " + std::to_string(players.size()) + " | Eggs: " + std::to_string(eggs.size()), 17, HUD_ACCENT, 15.f, y);

    for (std::size_t i = 0; i < maxPlayersShown; i++) {
        const Player &player = *players[i];
        drawLine(window, "#" + std::to_string(player._id) + " " + player._teamName
            + " | Lvl :" + std::to_string(player._level)
            + " | Orientation : " + std::to_string(player._orientation)
            + " | Food : " + std::to_string(player._food), 15, HUD_TEXT, 15.f, y);
    }
    if (players.size() > maxPlayersShown)
        drawLine(window, "+" + std::to_string(players.size() - maxPlayersShown) + " more players", 15, HUD_MUTED, 15.f, y);

    for (std::size_t i = 0; i < eggs.size(); i++) {
        const Egg &egg = *eggs[i];
        drawLine(window, "Egg #" + std::to_string(egg._id) + " from player #" + std::to_string(egg._playerId), 15, HUD_TEXT, 15.f, y);
    }
}

void Hud::renderBroadcasts(sf::RenderWindow &window, const GameState &state, float windowWidth, float windowHeight)
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

void Hud::renderCursor(sf::RenderWindow &window, float windowWidth, float windowHeight)
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

bool Hud::isTileValid(const GameState &state, const std::optional<sf::Vector2i> &selectedTile) const
{
    if (!selectedTile)
        return false;
    return selectedTile->x >= 0 && selectedTile->x < state.getWidth()
        && selectedTile->y >= 0 && selectedTile->y < state.getHeight();
}
