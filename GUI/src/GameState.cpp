#include "../include/GameState.hpp"
#include "GameState.hpp"

GameState::GameState(): _width(0), _height(0), _timeUnit(100),
      _gameOver(false)
{
}

int GameState::getWidth() const
{
      return _width;
}

int GameState::getHeight() const
{     
      return _height;
}

Tile &GameState::getTile(int x, int y)
{
    return _map[y][x];
}

const Tile &GameState::getTile(int x, int y) const
{
    return _map[y][x];
}

std::vector<Player> &GameState::getPlayers()
{
    return _players;
}

const std::vector<Player> &GameState::getPlayers() const
{
    return _players;
}

Player *GameState::findPlayer(uint32_t id)
{
    for (auto &p : _players)
        if (p._id == id)
            return &p;
    return nullptr;
}

const std::vector<std::string> &GameState::getTeams() const
{
    return _teams;
}

int GameState::getTimeUnit() const
{
      return _timeUnit;
}

bool GameState::isGameOver() const
{
      return _gameOver;
}

const std::string &GameState::getWinner() const
{
      return _winner;
}

std::vector<std::vector<Tile>>& GameState::getMap()
{
    return _map;
}

void GameState::setSize(int width, int height)
{
    _width = width;
    _height = height;
    _map.assign(_height, std::vector<Tile>(_width));
}

void GameState::addTeam(const std::string &name)
{
    _teams.push_back(name);
}

void GameState::addPlayer(const Player &player)
{
    _players.push_back(player);
}

void GameState::setTimeUnit(int t)
{
    _timeUnit = t;
}

void GameState::setGameOver(const std::string &winner)
{
    _winner = winner;
    _gameOver = true;
}

void GameState::lock()
{
      _mutex.lock();
}

void GameState::unlock()
{
      _mutex.unlock();
}
