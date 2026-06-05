#pragma once

#include <array>
#include <cstddef>
#include <vector>

#include "core/Player.hpp"
#include "core/Ressource.hpp"
#include "core/Tile.hpp"

class Core {
public:
    static constexpr std::size_t DefaultMapWidth = 10;
    static constexpr std::size_t DefaultMapHeight = 10;
    using Map = std::array<std::array<Tile, DefaultMapWidth>, DefaultMapHeight>;

    Core() = default;

    Map &getMap();
    const Map &getMap() const;
    std::vector<Player> &getPlayers();
    const std::vector<Player> &getPlayers() const;
    std::vector<Ressource> &getRessources();
    const std::vector<Ressource> &getRessources() const;

private:
    Map _map;
    std::vector<Player> _players;
    std::vector<Ressource> _ressources;
};
