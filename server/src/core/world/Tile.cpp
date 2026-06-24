#include "core/world/Tile.hpp"

std::vector<std::shared_ptr<Player>> &Tile::getPlayers()
{
    return _players;
}

const std::vector<std::shared_ptr<Player>> &Tile::getPlayers() const
{
    return _players;
}

std::vector<std::shared_ptr<Ressource>> &Tile::getRessources()
{
    return _ressources;
}

const std::vector<std::shared_ptr<Ressource>> &Tile::getRessources() const
{
    return _ressources;
}
