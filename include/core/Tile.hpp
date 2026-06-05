#pragma once

#include <memory>
#include <vector>

#include "core/Player.hpp"
#include "core/Ressource.hpp"

class Tile {
public:
    Tile() = default;
    ~Tile() = default;

    Tile(const Tile &) = delete;
    Tile &operator=(const Tile &) = delete;
    Tile(Tile &&) = default;
    Tile &operator=(Tile &&) = default;

    std::vector<std::unique_ptr<Player>> &getPlayers();
    const std::vector<std::unique_ptr<Player>> &getPlayers() const;
    std::vector<std::unique_ptr<Ressource>> &getRessources();
    const std::vector<std::unique_ptr<Ressource>> &getRessources() const;

private:
    std::vector<std::unique_ptr<Player>> _players;
    std::vector<std::unique_ptr<Ressource>> _ressources;
};
