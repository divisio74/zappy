#pragma once

#include <cstddef>
#include <memory>
#include <unordered_map>
#include <vector>

#include "core/entities/Egg.hpp"
#include "core/entities/Player.hpp"
#include "core/entities/Ressource.hpp"
#include "core/entities/Team.hpp"
#include "core/world/Tile.hpp"

struct WorldData {
    double freq;
    std::vector<std::vector<Tile>> map;
    std::unordered_map<std::size_t, Player> players;
    std::vector<std::shared_ptr<Ressource>> ressources;
    std::unordered_map<ResourceType, int> ressourcesQt;
    std::vector<std::shared_ptr<Egg>> eggs;
    std::unordered_map<std::size_t, Team> teams;
    Vector2 size;
};
