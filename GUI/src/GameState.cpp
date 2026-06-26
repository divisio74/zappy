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

std::map<int, Player> &GameState::getPlayers()
{
    return _players;
}

const std::map<int, Player> &GameState::getPlayers() const
{
    return _players;
}

Player *GameState::findPlayer(int id)
{
    auto player = _players.find(id);
    if (player != _players.end())
        return &player->second;
    return nullptr;
}

std::vector<Egg> &GameState::getEggs()
{
    return _eggs;
}

const std::vector<Egg> &GameState::getEggs() const
{
    return _eggs;
}

Egg *GameState::findEgg(int id)
{
    for (auto &e : _eggs) {
        if (e._id == id)
            return &e;
    }
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
    _players[player._id] = player;
}

void GameState::addEgg(const Egg &egg)
{
    _eggs.push_back(egg);
}

void GameState::removeEgg(int id)
{
    for (std::size_t i = 0; i < _eggs.size(); i++) {
        if (_eggs[i]._id == id) {
            _eggs.erase(_eggs.begin() + i);
            return;
        }
    }
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
