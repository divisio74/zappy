#pragma once

#include <vector>

#include "core/entities/Egg.hpp"
#include "core/entities/Player.hpp"
#include "core/entities/Ressource.hpp"
#include "core/entities/Team.hpp"
#include "core/world/Tile.hpp"

struct WorldData {
    std::vector<std::vector<Tile>> map;
    std::vector<Player> players;
    std::vector<Ressource> ressources;
    std::vector<Egg> eggs;
    std::vector<Team> teams;
};
