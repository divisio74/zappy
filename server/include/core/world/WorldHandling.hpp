#pragma once

#include <cstddef>
#include <string>
#include <vector>

#include "core/world/WorldData.hpp"

class WorldHandling {
public:
    explicit WorldHandling(WorldData &data);

    void createMap(std::size_t width, std::size_t height,
        const std::vector<std::string> &teamNames, std::size_t places);
    const std::vector<std::vector<Tile>> &getMap() const;
    void move();

private:
    void createTeams(const std::vector<std::string> &teamNames, std::size_t places);
    Vector2 getRandomPosition() const;
    void initRessourcesQt();
    void spawnRessources();
    void createEggs(std::size_t teamId, std::size_t places);

    WorldData &_data;
};
