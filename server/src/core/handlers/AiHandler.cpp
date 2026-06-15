#include "core/handlers/AiHandler.hpp"
#include <string>
#include <sstream>
#include "AiHandler.hpp"

static const std::unordered_map<std::string, int> ACTION_COSTS = {
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

void AiHandler::addMessage(size_t id, std::string message)
{
    _messageQueue.push({id, message + "\n"});
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
            enqueueCommand(player, command);
        } else {
            addMessage(playerId, "ko");
        }
    }
}

void AiHandler::run()
{
    for (auto& p : _world.players) {
        auto& queue = p.getPlayerAction();
        playerAction action = queue.front();
        auto now = std::chrono::steady_clock::now();
        if (action.executeAt < now) {
            queue.pop();
            executeCommand(p.getId(), action.command);
            p.setWaitTime(p.getWaitTime() - action.time);
        }
    }
}

void AiHandler::executeCommand(size_t playerId, const std::string &command)
{
    auto player = findPlayer(playerId);

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
    for (auto& player : _world.players) {
        if (player.getId() == playerId)
            return player;
    }
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
    addMessage(player.getId(), "ok");
}

void AiHandler::execRight(Player &player)
{
    int dir = static_cast<int>(player.getDirection());

    player.setDirection(static_cast<Direction>((dir + 1) % 4));
    addMessage(player.getId(), "ok");
}

void AiHandler::execLeft(Player &player)
{
    int dir = static_cast<int>(player.getDirection());
    dir = (dir + 3) % 4;
    player.setDirection(static_cast<Direction>(dir));
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

            const Tile &tile = _world.map[pos.x][pos.y];

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
    addMessage(player.getId(), std::to_string(_world.eggs.size()));
}

void AiHandler::execFork(Player &player)
{
    
}
