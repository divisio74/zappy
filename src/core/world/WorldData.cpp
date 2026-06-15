#include "core/world/WorldData.hpp"

WorldData::WorldData(const std::vector<std::string> &names, std::size_t x,
                     std::size_t y)
    : _map(y),
      _width(x),
      _height(y)
{
    for (std::size_t row = 0; row < y; ++row) {
        for (std::size_t column = 0; column < x; ++column)
            _map[row].push_back(Tile());
    }
    for (std::size_t id = 0; id < names.size(); ++id)
        _teams.emplace(names[id], Team(id, names[id]));
}

std::vector<std::vector<Tile>> &WorldData::getMap()
{
    return _map;
}

const std::vector<std::vector<Tile>> &WorldData::getMap() const
{
    return _map;
}

std::size_t WorldData::getWidth() const
{
    return _width;
}

std::size_t WorldData::getHeight() const
{
    return _height;
}

std::unordered_map<std::size_t, Player> &WorldData::getPlayers()
{
    return _players;
}

const std::unordered_map<std::size_t, Player> &WorldData::getPlayers() const
{
    return _players;
}

std::vector<Ressource> &WorldData::getRessources()
{
    return _ressources;
}

const std::vector<Ressource> &WorldData::getRessources() const
{
    return _ressources;
}

std::vector<Egg> &WorldData::getEggs()
{
    return _eggs;
}

const std::vector<Egg> &WorldData::getEggs() const
{
    return _eggs;
}

std::unordered_map<std::string, Team> &WorldData::getTeams()
{
    return _teams;
}

const std::unordered_map<std::string, Team> &WorldData::getTeams() const
{
    return _teams;
}
