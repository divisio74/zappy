#include "core/Core.hpp"

Core::Map &Core::getMap()
{
    return _map;
}

const Core::Map &Core::getMap() const
{
    return _map;
}

std::vector<Player> &Core::getPlayers()
{
    return _players;
}

const std::vector<Player> &Core::getPlayers() const
{
    return _players;
}

std::vector<Ressource> &Core::getRessources()
{
    return _ressources;
}

const std::vector<Ressource> &Core::getRessources() const
{
    return _ressources;
}
