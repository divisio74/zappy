#include "core/handlers/AiHandler.hpp"

#include <string>
#include <sstream>

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
            command.rfind("Broadcast ", 0) == 0 ||
            command.rfind("Take ", 0) == 0 ||
            command.rfind("Set ", 0) == 0)
        {
            if (command == "Incantation" && !canIncantation(player)) {
                addMessage(player.getId(), "ko");
                return;
            }
            enqueueCommand(player, command);
        } else {
            addMessage(playerId, "error");
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
            player.setWaitTime(player.getWaitTime() - action.time);
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

    const EvolutionCost &cost = it->second;
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
    if (player.getPlayerAction().size() >= 10) {
        addMessage(player.getId(), "ko");
        return;
    }

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

            for (size_t i = 0; i < tile.getPlayers().size(); ++i) {
                if (!firstObj)
                    response << " ";

                response << "player";
                firstObj = false;
            }
        }
    }

    response << "]\n";
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
    return;
}

void AiHandler::execIncantation(Player &player)
{
    return;
}

void AiHandler::execBroadcast(Player &player, const std::string &text)
{
    queueGuiEvent({.type = GuiEventType::Broadcast,
        .playerId = player.getId(),
        .message = text});
    addMessage(player.getId(), "ok");
}

void AiHandler::execTake(Player &player, const std::string &object)
{
    return;
}

void AiHandler::execSet(Player &player, const std::string &object)
{
    static_cast<void>(player);
    static_cast<void>(object);
    return;
}
