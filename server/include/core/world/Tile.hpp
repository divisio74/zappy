#pragma once

#include <memory>
#include <vector>

#include "core/entities/Egg.hpp"
#include "core/entities/Player.hpp"
#include "core/entities/Ressource.hpp"

class Tile {
public:
    Tile() = default;
    ~Tile() = default;

    Tile(const Tile &) = delete;
    Tile &operator=(const Tile &) = delete;
    Tile(Tile &&) = default;
    Tile &operator=(Tile &&) = default;

    std::vector<std::shared_ptr<Player>> &getPlayers();
    const std::vector<std::shared_ptr<Player>> &getPlayers() const;
    std::vector<std::shared_ptr<Ressource>> &getRessources();
    const std::vector<std::shared_ptr<Ressource>> &getRessources() const;

private:
    std::vector<std::shared_ptr<Player>> _players;
    std::vector<std::shared_ptr<Ressource>> _ressources;
};
