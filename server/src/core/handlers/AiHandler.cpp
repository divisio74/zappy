#include "core/handlers/AiHandler.hpp"

#include <algorithm>
#include <memory>
#include <string>
#include <sstream>

namespace {
bool getResourceType(const std::string &name, ResourceType &type)
{
    if (name == "food")
        type = ResourceType::Food;
    else if (name == "linemate")
        type = ResourceType::Linemate;
    else if (name == "deraumere")
        type = ResourceType::Deraumere;
    else if (name == "sibur")
        type = ResourceType::Sibur;
    else if (name == "mendiane")
        type = ResourceType::Mendiane;
    else if (name == "phiras")
        type = ResourceType::Phiras;
    else if (name == "thystame")
        type = ResourceType::Thystame;
    else
        return false;
    return true;
}

bool removeResourceFromWorld(
    std::vector<std::shared_ptr<Ressource>> &tileResources,
    std::vector<std::shared_ptr<Ressource>> &worldResources,
    ResourceType type)
{
    auto resource = std::find_if(tileResources.begin(), tileResources.end(),
        [type](const std::shared_ptr<Ressource> &resource) {
            return resource->getType() == type;
        });

    if (resource == tileResources.end())
        return false;

    const std::size_t resourceId = (*resource)->getId();
    tileResources.erase(resource);
    worldResources.erase(std::remove_if(worldResources.begin(), worldResources.end(),
        [resourceId](const std::shared_ptr<Ressource> &resource) {
            return resource->getId() == resourceId;
        }), worldResources.end());
    return true;
}

bool consumeResources(
    std::vector<std::shared_ptr<Ressource>> &tileResources,
    std::vector<std::shared_ptr<Ressource>> &worldResources,
    ResourceType type,
    int quantity)
{
    for (int i = 0; i < quantity; i++) {
        if (!removeResourceFromWorld(tileResources, worldResources, type))
            return false;
    }
    return true;
}

bool removeResourceFromInventory(Inventory &inventory, ResourceType type)
{
    int *quantity = nullptr;

    switch (type) {
        case ResourceType::Food:
            quantity = &inventory.food;
            break;
        case ResourceType::Linemate:
            quantity = &inventory.linemate;
            break;
        case ResourceType::Deraumere:
            quantity = &inventory.deraumere;
            break;
        case ResourceType::Sibur:
            quantity = &inventory.sibur;
            break;
        case ResourceType::Mendiane:
            quantity = &inventory.mendiane;
            break;
        case ResourceType::Phiras:
            quantity = &inventory.phiras;
            break;
        case ResourceType::Thystame:
            quantity = &inventory.thystame;
            break;
    }
    if (quantity == nullptr || *quantity <= 0)
        return false;
    (*quantity)--;
    return true;
}

std::size_t getNextResourceId(const std::vector<std::shared_ptr<Ressource>> &resources)
{
    std::size_t nextId = 0;

    for (const auto &resource : resources)
        nextId = std::max(nextId, resource->getId() + 1);
    return nextId;
}

int getWrappedDelta(int from, int to, int size)
{
    int delta = to - from;

    if (size <= 0)
        return delta;
    if (delta > size / 2)
        delta -= size;
    else if (delta < -size / 2)
        delta += size;
    return delta;
}

int getAbsoluteSector(int dx, int dy)
{
    if (dx == 0 && dy < 0)
        return 0;
    if (dx < 0 && dy < 0)
        return 1;
    if (dx < 0 && dy == 0)
        return 2;
    if (dx < 0 && dy > 0)
        return 3;
    if (dx == 0 && dy > 0)
        return 4;
    if (dx > 0 && dy > 0)
        return 5;
    if (dx > 0 && dy == 0)
        return 6;
    return 7;
}

int getBroadcastDirection(const Player &receiver, const Player &sender, const Vector2 &worldSize)
{
    const int dx = getWrappedDelta(receiver.getCoordinate().x, sender.getCoordinate().x, worldSize.x);
    const int dy = getWrappedDelta(receiver.getCoordinate().y, sender.getCoordinate().y, worldSize.y);

    if (dx == 0 && dy == 0)
        return 0;

    const int absoluteSector = getAbsoluteSector(dx, dy);
    const int receiverFrontSector = (8 - static_cast<int>(receiver.getDirection()) * 2) % 8;
    return ((absoluteSector - receiverFrontSector + 8) % 8) + 1;
}

}

const std::unordered_map<std::string, int> AiHandler::ACTION_COSTS = {
    {"Forward",     7},
    {"Right",       7},
    {"Left",        7},
    {"Look",        7},
    {"Inventory",   1},
    {"Broadcast",   7},
    {"Fork",        42},
    {"Eject",       7},
    {"Incantation", 300},
    {"Take",        7},
    {"Set",         7},
};

AiHandler::AiHandler(WorldData &world, std::function<void(const GuiEvent&)> queueGuiEvent)
    : _world(world),
      _queueGuiEvent(queueGuiEvent)
{
}

void AiHandler::addMessage(size_t id, std::string message)
{
    _messageQueue.push({id, message + "\n"});
}

void AiHandler::queueGuiEvent(const GuiEvent &event)
{
    if (_queueGuiEvent)
        _queueGuiEvent(event);
}

std::vector<message> AiHandler::getMessages()
{
    std::vector<message> messages;

    while (!_messageQueue.empty()) {
        messages.push_back(_messageQueue.front());
        _messageQueue.pop();
    }
    return messages;
}

void AiHandler::parseCommand(size_t playerId, std::vector<std::string> &commands)
{
    auto& player = _world.players.at(playerId);

    for (auto& command : commands) {
        if (command == "Forward"    ||
            command == "Right"      ||
            command == "Left"       ||
            command == "Look"       ||
            command == "Inventory"  ||
            command == "Connect_nbr"||
            command == "Fork"       ||
            command == "Eject"      ||
            command == "Incantation"||
            (command.rfind("Broadcast ", 0) == 0 && command.size() > 10) ||
            command.rfind("Take ", 0) == 0 ||
            command.rfind("Set ", 0) == 0)
        {
            if (command == "Incantation" && !canIncantation(player)) {
                addMessage(player.getId(), "ko");
                return;
            }
            if (command == "Incantation")
                addMessage(player.getId(), "Elevation underway");
            enqueueCommand(player, command);
        } else {
            addMessage(playerId, "ko");
        }
    }
}

void AiHandler::run()
{
    for (auto& [id, player] : _world.players) {
        auto& queue = player.getPlayerAction();
        if (queue.empty())
            continue;
        playerAction action = queue.front();
        auto now = std::chrono::steady_clock::now();
        if (action.executeAt < now) {
            queue.pop();
            executeCommand(id, action.command);
        }
    }
}

void AiHandler::executeCommand(size_t playerId, const std::string &command)
{
    Player &player = findPlayer(playerId);

    if (command == "Forward")     return execForward(player);
    if (command == "Right")       return execRight(player);
    if (command == "Left")        return execLeft(player);
    if (command == "Look")        return execLook(player);
    if (command == "Inventory")   return execInventory(player);
    if (command == "Connect_nbr") return execConnectNbr(player);
    if (command == "Fork")        return execFork(player);
    if (command == "Eject")       return execEject(player);
    if (command == "Incantation") return execIncantation(player);
    if (command.rfind("Broadcast ", 0) == 0)
        return execBroadcast(player, command.substr(10));
    if (command.rfind("Take ", 0) == 0)
        return execTake(player, command.substr(5));
    if (command.rfind("Set ", 0) == 0)
        return execSet(player, command.substr(4));
    addMessage(playerId, "ko");
}

Player &AiHandler::findPlayer(size_t playerId) const
{
    return _world.players.at(playerId);
}

Vector2 AiHandler::normalizePos(Vector2 pos) const
{
    while (pos.x < 0)
        pos.x += _world.size.x;
    pos.x = pos.x % _world.size.x;
    while (pos.y < 0)
        pos.y += _world.size.y;
    pos.y = pos.y % _world.size.y;
    return pos;
}

bool AiHandler::canIncantation(const Player &player) const
{
    int level = player.getLevel();
    auto it = evolutionCosts.find(level);
    if (it == evolutionCosts.end())
        return false;

    auto &cost = it->second;
    auto &coord = player.getCoordinate();
    auto &tileResources = _world.map[coord.y][coord.x].getRessources();

    int linemate = 0, deraumere = 0, sibur = 0, mendiane = 0, phiras = 0, thystame = 0;
    for (const auto &r : tileResources) {
        switch (r->getType()) {
            case ResourceType::Linemate: linemate++; break;
            case ResourceType::Deraumere: deraumere++; break;
            case ResourceType::Sibur: sibur++; break;
            case ResourceType::Mendiane: mendiane++; break;
            case ResourceType::Phiras: phiras++; break;
            case ResourceType::Thystame: thystame++; break;
            default: break;
        }
    }

    if (linemate < cost.linemate) return false;
    if (deraumere < cost.deraumere) return false;
    if (sibur < cost.sibur) return false;
    if (mendiane < cost.mendiane) return false;
    if (phiras < cost.phiras) return false;
    if (thystame < cost.thystame) return false;

    int sameLevelCount = countPlayersAtLevel(coord, level);
    if (sameLevelCount < cost.nbPlayers)
        return false;

    return true;
}

int AiHandler::countPlayersAtLevel(const Vector2 &coord, int level) const
{
    int count = 0;
    for (const auto &[id, player] : _world.players) {
        if (player.getCoordinate().x == coord.x &&
            player.getCoordinate().y == coord.y &&
            player.getLevel() == level)
            count++;
    }
    return count;
}

void AiHandler::enqueueCommand(Player &player, const std::string &command)
{
    if (player.getPlayerAction().size() >= 10)
        return;

    int cost = 0;
    for (const auto& [key, val] : ACTION_COSTS) {
        if (command.rfind(key, 0) == 0) {
            cost = val;
            break;
        }
    }

    auto ms = std::chrono::milliseconds{
        static_cast<std::chrono::milliseconds::rep>(
            cost * 1000 / _world.freq
        )
    };
    auto executeAt = std::chrono::steady_clock::now() + ms;

    playerAction action{
        command,
        executeAt,
        ms
    };

    player.getPlayerAction().push(action);
}

void AiHandler::execForward(Player &player)
{
    Vector2 pos = player.getCoordinate();
    switch (player.getDirection())
    {
    case Direction::North:
        pos.y -= 1;
        break;

    case Direction::East:
        pos.x += 1;
        break;

    case Direction::South:
        pos.y += 1;
        break;

    case Direction::West:
        pos.x -= 1;
        break;

    default:
        break;
    }
    pos = normalizePos(pos);
    player.setCoordinate(pos);
    queueGuiEvent({.type = GuiEventType::PlayerPosition,
        .playerId = player.getId()});
    addMessage(player.getId(), "ok");
}

void AiHandler::execRight(Player &player)
{
    int dir = static_cast<int>(player.getDirection());

    player.setDirection(static_cast<Direction>((dir + 1) % 4));
    queueGuiEvent({.type = GuiEventType::PlayerPosition,
        .playerId = player.getId()});
    addMessage(player.getId(), "ok");
}

void AiHandler::execLeft(Player &player)
{
    int dir = static_cast<int>(player.getDirection());
    dir = (dir + 3) % 4;
    player.setDirection(static_cast<Direction>(dir));
    queueGuiEvent({.type = GuiEventType::PlayerPosition,
        .playerId = player.getId()});
    addMessage(player.getId(), "ok");
}

void AiHandler::execLook(Player &player)
{
    std::stringstream response;

    response << "[";

    bool first = true;
    int level = player.getLevel();

    for (int dist = 0; dist <= level; ++dist) {
        for (int offset = -dist; offset <= dist; ++offset) {
            Vector2 pos = player.getCoordinate();

            switch (player.getDirection()) {
                case Direction::North:
                    pos.x += offset;
                    pos.y -= dist;
                    break;

                case Direction::South:
                    pos.x -= offset;
                    pos.y += dist;
                    break;

                case Direction::East:
                    pos.x += dist;
                    pos.y += offset;
                    break;

                case Direction::West:
                    pos.x -= dist;
                    pos.y -= offset;
                    break;
            }

            pos = normalizePos(pos);

            const Tile &tile = _world.map[pos.y][pos.x];

            if (!first)
                response << ",";

            first = false;
            bool firstObj = true;

            for (const auto &resource : tile.getRessources()) {
                if (!firstObj)
                    response << " ";

                response << resource->toString();
                firstObj = false;
            }

            std::size_t playerCount = 0;

            for (const auto &[id, visiblePlayer] : _world.players) {
                static_cast<void>(id);
                if (visiblePlayer.getCoordinate().x == pos.x
                    && visiblePlayer.getCoordinate().y == pos.y)
                    playerCount++;
            }
            for (std::size_t i = 0; i < playerCount; ++i) {
                if (!firstObj)
                    response << " ";

                response << "player";
                firstObj = false;
            }
        }
    }

    response << "]";
    addMessage(player.getId(), response.str());
}

void AiHandler::execInventory(Player &player)
{
    addMessage(player.getId(), player.getInventory().toString());
}

void AiHandler::execConnectNbr(Player &player)
{
    Team& team = _world.teams[player.getTeamId()];
    addMessage(player.getId(), std::to_string(team.getMaxSlot() - team.getUsedSlot()));
}

void AiHandler::execFork(Player &player)
{
    Team& team = _world.teams[player.getTeamId()];
    const Vector2 &position = player.getCoordinate();
    std::size_t eggId = _world.nextEggId++;

    team.setMaxSlot(team.getMaxSlot() + 1);
    _world.eggs.push_back(Egg(position, player.getTeamId(), eggId));
    queueGuiEvent({.type = GuiEventType::EggLaying,
        .playerId = player.getId()});
    queueGuiEvent({.type = GuiEventType::EggLaid,
        .x = position.x,
        .y = position.y,
        .playerId = player.getId(),
        .eggId = eggId});
    addMessage(player.getId(), "ok");
}

void AiHandler::execEject(Player &ejector)
{
    Vector2 targetPos = ejector.getCoordinate();

    switch (ejector.getDirection()) {
        case Direction::North:
            targetPos.y -= 1;
            break;
        case Direction::East:
            targetPos.x += 1;
            break;
        case Direction::South:
            targetPos.y += 1;
            break;
        case Direction::West:
            targetPos.x -= 1;
            break;
    }
    targetPos = normalizePos(targetPos);

    bool ejected = false;
    const Vector2 ejectorPos = ejector.getCoordinate();

    for (auto &[id, player] : _world.players) {
        if (id == ejector.getId())
            continue;
        if (player.getCoordinate().x != ejectorPos.x
            || player.getCoordinate().y != ejectorPos.y)
            continue;
        player.setCoordinate(targetPos);
        Player source(ejectorPos, Direction::North, 0, 0);
        const std::string ejectMessage = "eject: "
            + std::to_string(getBroadcastDirection(player, source, _world.size));

        addMessage(player.getId(), ejectMessage);
        ejected = true;
    }

    auto &eggs = _world.eggs;
    auto removed = std::remove_if(eggs.begin(), eggs.end(),
        [&ejectorPos](const Egg &egg) {
            return egg.getCoordinate().x == ejectorPos.x
                && egg.getCoordinate().y == ejectorPos.y;
        });
    if (removed != eggs.end()) {
        eggs.erase(removed, eggs.end());
        ejected = true;
    }

    queueGuiEvent({.type = GuiEventType::Expulsion,
        .playerId = ejector.getId()});
    addMessage(ejector.getId(), "ok");
}

void AiHandler::execIncantation(Player &player)
{
    const Vector2 coord = player.getCoordinate();

    if (!canIncantation(player)) {
        addMessage(player.getId(), "ko");
        queueGuiEvent({.type = GuiEventType::IncantationEnd,
            .x = coord.x,
            .y = coord.y,
            .result = 0});
        return;
    }
    const int currentLevel = player.getLevel();
    auto &cost = evolutionCosts.at(currentLevel);
    auto &tileResources = _world.map[coord.y][coord.x].getRessources();
    std::vector<std::size_t> playerIds;

    for (auto &[id, target] : _world.players) {
        if (target.getCoordinate().x == coord.x
            && target.getCoordinate().y == coord.y
            && target.getLevel() == currentLevel)
            playerIds.push_back(id);
    }
    queueGuiEvent({.type = GuiEventType::IncantationStart,
        .x = coord.x,
        .y = coord.y,
        .level = currentLevel,
        .playerIds = playerIds});
    consumeResources(tileResources, _world.ressources, ResourceType::Linemate, cost.linemate);
    consumeResources(tileResources, _world.ressources, ResourceType::Deraumere, cost.deraumere);
    consumeResources(tileResources, _world.ressources, ResourceType::Sibur, cost.sibur);
    consumeResources(tileResources, _world.ressources, ResourceType::Mendiane, cost.mendiane);
    consumeResources(tileResources, _world.ressources, ResourceType::Phiras, cost.phiras);
    consumeResources(tileResources, _world.ressources, ResourceType::Thystame, cost.thystame);

    const int newLevel = currentLevel + 1;
    for (auto &[id, target] : _world.players) {
        if (target.getCoordinate().x != coord.x
            || target.getCoordinate().y != coord.y
            || target.getLevel() != currentLevel)
            continue;
        target.setLevel(newLevel);
        addMessage(target.getId(), "Current level: " + std::to_string(newLevel));
        queueGuiEvent({.type = GuiEventType::PlayerLevel,
            .playerId = target.getId()});
    }
    queueGuiEvent({.type = GuiEventType::IncantationEnd,
        .x = coord.x,
        .y = coord.y,
        .result = 1});
    queueGuiEvent({.type = GuiEventType::TileContent,
        .x = coord.x,
        .y = coord.y});
}

void AiHandler::execBroadcast(Player &player, const std::string &text)
{
    for (auto &[id, receiver] : _world.players) {
        if (id == player.getId())
            continue;

        const int direction = getBroadcastDirection(receiver, player, _world.size);
        addMessage(receiver.getId(),
            "message " + std::to_string(direction) + ", " + text);
    }
    queueGuiEvent({.type = GuiEventType::Broadcast,
        .playerId = player.getId(),
        .message = text});
    addMessage(player.getId(), "ok");
}
void AiHandler::execTake(Player &player, const std::string &object)
{
    ResourceType type;

    if (!getResourceType(object, type)) {
        addMessage(player.getId(), "ko");
        return;
    }
    const Vector2 &pos = player.getCoordinate();
    auto &tileResources = _world.map[pos.y][pos.x].getRessources();
    auto tileResource = std::find_if(tileResources.begin(), tileResources.end(),
        [type](const std::shared_ptr<Ressource> &resource) {
            return resource->getType() == type;
        });

    if (tileResource == tileResources.end()) {
        addMessage(player.getId(), "ko");
        return;
    }

    const std::size_t resourceId = (*tileResource)->getId();

    player.addRessource(type);
    tileResources.erase(tileResource);

    auto &resources = _world.ressources;
    resources.erase(std::remove_if(resources.begin(), resources.end(),
        [resourceId](const std::shared_ptr<Ressource> &resource) {
            return resource->getId() == resourceId;
        }), resources.end());

    queueGuiEvent({.type = GuiEventType::ResourceCollect,
        .playerId = player.getId(),
        .resource = type});
    queueGuiEvent({.type = GuiEventType::PlayerInventory,
        .playerId = player.getId()});
    queueGuiEvent({.type = GuiEventType::TileContent,
        .x = pos.x,
        .y = pos.y});
    addMessage(player.getId(), "ok");
}

void AiHandler::execSet(Player &player, const std::string &object)
{
    ResourceType type;

    if (!getResourceType(object, type)
        || !removeResourceFromInventory(player.getInventory(), type)) {
        addMessage(player.getId(), "ko");
        return;
    }

    const Vector2 pos = player.getCoordinate();
    auto resource = std::make_shared<Ressource>(type, getNextResourceId(_world.ressources));

    _world.ressources.push_back(resource);
    _world.map[pos.y][pos.x].getRessources().push_back(resource);
    queueGuiEvent({.type = GuiEventType::ResourceDrop,
        .playerId = player.getId(),
        .resource = type});
    queueGuiEvent({.type = GuiEventType::PlayerInventory,
        .playerId = player.getId()});
    queueGuiEvent({.type = GuiEventType::TileContent,
        .x = pos.x,
        .y = pos.y});
    addMessage(player.getId(), "ok");
}
