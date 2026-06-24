#include "core/world/WorldHandling.hpp"

#include <array>
#include <cstdlib>
#include <ctime>
#include <memory>
#include <utility>

namespace {
const std::array<std::pair<ResourceType, double>, 7> RESSOURCES_QT = {{
    {ResourceType::Food, 0.5},
    {ResourceType::Linemate, 0.3},
    {ResourceType::Deraumere, 0.15},
    {ResourceType::Sibur, 0.1},
    {ResourceType::Mendiane, 0.1},
    {ResourceType::Phiras, 0.08},
    {ResourceType::Thystame, 0.05},
}};
}

WorldHandling::WorldHandling(WorldData &data)
    : _data(data)
{
    std::srand(static_cast<unsigned int>(std::time(nullptr)));
}

void WorldHandling::createMap(std::size_t width, std::size_t height,
    const std::vector<std::string> &teamNames, std::size_t places)
{
    _data.size.x = static_cast<int>(width);
    _data.size.y = static_cast<int>(height);
    for (std::size_t y = 0; y < height; y++) {
        _data.map.push_back(std::vector<Tile>());
        for (std::size_t x = 0; x < width; x++)
            _data.map[y].push_back(Tile());
    }
    initRessourcesQt();
    spawnRessources();
    createTeams(teamNames, places);
}

void WorldHandling::createTeams(const std::vector<std::string> &teamNames, std::size_t places)
{
    for (std::size_t id = 0; id < teamNames.size(); id++) {
        Team team(id, teamNames[id], places);

        _data.teams[id] = team;
        createEggs(id, places);
    }
}

Vector2 WorldHandling::getRandomPosition() const
{
    if (_data.size.x <= 0 || _data.size.y <= 0)
        return Vector2();

    return Vector2{std::rand() % _data.size.x, std::rand() % _data.size.y};
}

void WorldHandling::initRessourcesQt()
{
    int mapSize = _data.size.x * _data.size.y;

    for (const auto &[type, density] : RESSOURCES_QT)
        _data.ressourcesQt[type] = static_cast<int>(mapSize * density);
}

void WorldHandling::spawnRessources()
{
    for (const auto &[type, density] : RESSOURCES_QT) {
        static_cast<void>(density);
        int quantity = _data.ressourcesQt[type];

        for (int i = 0; i < quantity; i++) {
            Vector2 position = getRandomPosition();
            auto ressource = std::make_shared<Ressource>(position, type, _data.ressources.size());

            _data.ressources.push_back(ressource);
            if (position.y >= 0 && static_cast<std::size_t>(position.y) < _data.map.size()
                && position.x >= 0 && static_cast<std::size_t>(position.x) < _data.map[position.y].size())
                _data.map[position.y][position.x].getRessources().push_back(ressource);
        }
    }
}

void WorldHandling::createEggs(std::size_t teamId, std::size_t places)
{
    for (std::size_t i = 0; i < places; i++) {
        Vector2 position = getRandomPosition();
        auto egg = std::make_shared<Egg>(position, teamId);

        _data.eggs.push_back(egg);
        if (position.y >= 0 && static_cast<std::size_t>(position.y) < _data.map.size()
            && position.x >= 0 && static_cast<std::size_t>(position.x) < _data.map[position.y].size())
            _data.map[position.y][position.x].getEggs().push_back(egg);
    }
}

const std::vector<std::vector<Tile>> &WorldHandling::getMap() const
{
    return _data.map;
}

void WorldHandling::move()
{
}
