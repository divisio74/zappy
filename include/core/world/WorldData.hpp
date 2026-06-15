#pragma once

#include <cstddef>
#include <string>
#include <unordered_map>
#include <vector>

#include "core/entities/Egg.hpp"
#include "core/entities/Player.hpp"
#include "core/entities/Ressource.hpp"
#include "core/entities/Team.hpp"
#include "core/world/Tile.hpp"

class WorldData {
public:
    WorldData(const std::vector<std::string> &names, std::size_t x,
              std::size_t y);

    std::vector<std::vector<Tile>> &getMap();
    const std::vector<std::vector<Tile>> &getMap() const;
    std::size_t getWidth() const;
    std::size_t getHeight() const;
    std::unordered_map<std::size_t, Player> &getPlayers();
    const std::unordered_map<std::size_t, Player> &getPlayers() const;
    std::vector<Ressource> &getRessources();
    const std::vector<Ressource> &getRessources() const;
    std::vector<Egg> &getEggs();
    const std::vector<Egg> &getEggs() const;
    std::unordered_map<std::string, Team> &getTeams();
    const std::unordered_map<std::string, Team> &getTeams() const;

private:
    std::vector<std::vector<Tile>> _map;
    std::size_t _width;
    std::size_t _height;
    std::unordered_map<std::size_t, Player> _players;
    std::vector<Ressource> _ressources;
    std::vector<Egg> _eggs;
    std::unordered_map<std::string, Team> _teams;
};
