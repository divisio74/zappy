#include "core/handlers/GuiHandler.hpp"

#include <array>
#include <sstream>

namespace {
const std::string UNKNOWN_COMMAND_RESPONSE = "suc";
const std::string BAD_PARAMETER_RESPONSE = "sbp";
constexpr int MAX_FREQUENCY = 1000;

enum class GuiCommand {
    MapSize,
    TileContent,
    MapContent,
    TeamNames,
    PlayerPosition,
    PlayerLevel,
    PlayerInventory,
    TimeUnit,
    TimeUnitModification,
    Unknown
};

const std::array<ResourceType, 7> RESOURCE_ORDER = {
    ResourceType::Food,
    ResourceType::Linemate,
    ResourceType::Deraumere,
    ResourceType::Sibur,
    ResourceType::Mendiane,
    ResourceType::Phiras,
    ResourceType::Thystame
};

std::vector<std::string> splitCommand(const std::string &command)
{
    std::vector<std::string> parts;
    std::istringstream stream(command);
    std::string part;

    while (stream >> part)
        parts.push_back(part);
    return parts;
}

GuiCommand getGuiCommand(const std::string &name)
{
    if (name == "msz")
        return GuiCommand::MapSize;
    if (name == "bct")
        return GuiCommand::TileContent;
    if (name == "mct")
        return GuiCommand::MapContent;
    if (name == "tna")
        return GuiCommand::TeamNames;
    if (name == "ppo")
        return GuiCommand::PlayerPosition;
    if (name == "plv")
        return GuiCommand::PlayerLevel;
    if (name == "pin")
        return GuiCommand::PlayerInventory;
    if (name == "sgt")
        return GuiCommand::TimeUnit;
    if (name == "sst")
        return GuiCommand::TimeUnitModification;
    return GuiCommand::Unknown;
}

bool parsePositiveNumber(const std::string &value, int &number)
{
    std::size_t position = 0;

    try {
        number = std::stoi(value, &position);
    } catch (const std::exception &) {
        return false;
    }
    return position == value.size() && number >= 0;
}

bool parsePlayerId(const std::string &value, std::size_t &playerId)
{
    int parsedId = 0;

    if (value.size() < 2 || value[0] != '#')
        return false;
    if (!parsePositiveNumber(value.substr(1), parsedId))
        return false;
    playerId = static_cast<std::size_t>(parsedId);
    return true;
}

int getResourceIndex(ResourceType type)
{
    for (std::size_t i = 0; i < RESOURCE_ORDER.size(); ++i) {
        if (RESOURCE_ORDER[i] == type)
            return static_cast<int>(i);
    }
    return 0;
}

int getGuiOrientation(Direction direction)
{
    switch (direction) {
    case Direction::North:
        return 1;
    case Direction::East:
        return 2;
    case Direction::South:
        return 3;
    case Direction::West:
        return 4;
    }
    return 1;
}
}

GuiHandler::GuiHandler(WorldData &worldData)
    : _worldData(worldData)
{
}

std::vector<std::string> GuiHandler::handleCommand(const std::string &command)
{
    std::vector<std::string> args = splitCommand(command);

    if (args.empty())
        return {UNKNOWN_COMMAND_RESPONSE};

    switch (getGuiCommand(args[0])) {
    case GuiCommand::MapSize:
        return {handleMapSize(args)};
    case GuiCommand::TileContent:
        return {handleTileContent(args)};
    case GuiCommand::MapContent:
        return handleMapContent(args);
    case GuiCommand::TeamNames:
        return handleTeamNames(args);
    case GuiCommand::PlayerPosition:
        return {handlePlayerPosition(args)};
    case GuiCommand::PlayerLevel:
        return {handlePlayerLevel(args)};
    case GuiCommand::PlayerInventory:
        return {handlePlayerInventory(args)};
    case GuiCommand::TimeUnit:
        return {handleTimeUnit(args)};
    case GuiCommand::TimeUnitModification:
        return {handleTimeUnitModification(args)};
    case GuiCommand::Unknown:
        return {UNKNOWN_COMMAND_RESPONSE};
    }
    return {UNKNOWN_COMMAND_RESPONSE};
}

const std::vector<std::vector<Tile>> &GuiHandler::getMap() const
{
    return _worldData.map;
}

std::string GuiHandler::formatMapSize() const
{
    std::ostringstream response;

    response << "msz " << _worldData.size.x << " " << _worldData.size.y;
    return response.str();
}

std::string GuiHandler::formatTileContent(int x, int y) const
{
    std::array<int, RESOURCE_ORDER.size()> quantities = {};
    std::ostringstream response;

    for (const std::shared_ptr<Ressource> &ressource : _worldData.map[y][x].getRessources()) {
        if (ressource != nullptr)
            quantities[getResourceIndex(ressource->getType())]++;
    }

    response << "bct " << x << " " << y;
    for (int quantity : quantities)
        response << " " << quantity;
    return response.str();
}

std::vector<std::string> GuiHandler::formatMapContent() const
{
    std::vector<std::string> responses;

    for (int y = 0; y < _worldData.size.y; ++y) {
        for (int x = 0; x < _worldData.size.x; ++x)
            responses.push_back(formatTileContent(x, y));
    }
    return responses;
}

std::vector<std::string> GuiHandler::formatTeamNames() const
{
    std::vector<std::string> responses;

    for (const auto &teamEntry : _worldData.teams)
        responses.push_back("tna " + teamEntry.second.getName());
    return responses;
}

std::string GuiHandler::formatPlayerConnection(const Player &player) const
{
    const Vector2 &position = player.getCoordinate();
    std::ostringstream response;
    std::string teamName;
    auto team = _worldData.teams.find(player.getTeamId());

    if (team != _worldData.teams.end())
        teamName = team->second.getName();
    response << "pnw #" << player.getId() << " " << position.x << " "
             << position.y << " " << getGuiOrientation(player.getDirection())
             << " " << player.getLevel() << " " << teamName;
    return response.str();
}

std::string GuiHandler::formatPlayerPosition(std::size_t playerId) const
{
    auto player = _worldData.players.find(playerId);
    std::ostringstream response;

    if (player == _worldData.players.end())
        return BAD_PARAMETER_RESPONSE;

    const Vector2 &position = player->second.getCoordinate();

    response << "ppo #" << playerId << " " << position.x << " "
             << position.y << " "
             << getGuiOrientation(player->second.getDirection());
    return response.str();
}

std::string GuiHandler::formatPlayerLevel(std::size_t playerId) const
{
    auto player = _worldData.players.find(playerId);
    std::ostringstream response;

    if (player == _worldData.players.end())
        return BAD_PARAMETER_RESPONSE;
    response << "plv #" << playerId << " " << player->second.getLevel();
    return response.str();
}

std::string GuiHandler::formatPlayerInventory(std::size_t playerId) const
{
    auto player = _worldData.players.find(playerId);
    std::ostringstream response;

    if (player == _worldData.players.end())
        return BAD_PARAMETER_RESPONSE;

    const Vector2 &position = player->second.getCoordinate();
    const Inventory &inventory = player->second.getInventory();

    response << "pin #" << playerId << " " << position.x << " " << position.y
             << " " << inventory.food << " " << inventory.linemate << " "
             << inventory.deraumere << " " << inventory.sibur << " "
             << inventory.mendiane << " " << inventory.phiras << " "
             << inventory.thystame;
    return response.str();
}

std::string GuiHandler::formatExpulsion(std::size_t playerId) const
{
    return "pex #" + std::to_string(playerId);
}

std::string GuiHandler::formatBroadcast(std::size_t playerId,
    const std::string &message) const
{
    return "pbc #" + std::to_string(playerId) + " " + message;
}

std::string GuiHandler::formatIncantationStart(int x, int y, int level,
    const std::vector<std::size_t> &playerIds) const
{
    std::ostringstream response;

    response << "pic " << x << " " << y << " " << level;
    for (std::size_t playerId : playerIds)
        response << " #" << playerId;
    return response.str();
}

std::string GuiHandler::formatIncantationEnd(int x, int y, int result) const
{
    std::ostringstream response;

    response << "pie " << x << " " << y << " " << result;
    return response.str();
}

std::string GuiHandler::formatEggLaying(std::size_t playerId) const
{
    return "pfk #" + std::to_string(playerId);
}

std::string GuiHandler::formatResourceDrop(std::size_t playerId,
    ResourceType type) const
{
    return "pdr #" + std::to_string(playerId) + " "
        + std::to_string(getResourceIndex(type));
}

std::string GuiHandler::formatResourceCollect(std::size_t playerId,
    ResourceType type) const
{
    return "pgt #" + std::to_string(playerId) + " "
        + std::to_string(getResourceIndex(type));
}

std::string GuiHandler::formatPlayerDeath(std::size_t playerId) const
{
    return "pdi #" + std::to_string(playerId);
}

std::string GuiHandler::formatEggLaid(std::size_t eggId,
    std::size_t playerId, int x, int y) const
{
    std::ostringstream response;

    response << "enw #" << eggId << " #" << playerId << " " << x << " " << y;
    return response.str();
}

std::string GuiHandler::formatEggConnection(std::size_t eggId) const
{
    return "ebo #" + std::to_string(eggId);
}

std::string GuiHandler::formatEggDeath(std::size_t eggId) const
{
    return "edi #" + std::to_string(eggId);
}

std::string GuiHandler::formatTimeUnit() const
{
    return "sgt " + std::to_string(_worldData.freq);
}

std::string GuiHandler::formatTimeUnitModification(int timeUnit) const
{
    return "sst " + std::to_string(timeUnit);
}

std::string GuiHandler::formatEndGame(const std::string &teamName) const
{
    return "seg " + teamName;
}

std::string GuiHandler::formatServerMessage(const std::string &message) const
{
    return "smg " + message;
}

std::string GuiHandler::handleMapSize(const std::vector<std::string> &args) const
{
    if (args.size() != 1)
        return BAD_PARAMETER_RESPONSE;
    return formatMapSize();
}

std::string GuiHandler::handleTileContent(const std::vector<std::string> &args) const
{
    int x = 0;
    int y = 0;

    if (args.size() != 3 || !parsePositiveNumber(args[1], x)
        || !parsePositiveNumber(args[2], y) || !isPositionValid(x, y))
        return BAD_PARAMETER_RESPONSE;
    return formatTileContent(x, y);
}

std::vector<std::string> GuiHandler::handleMapContent(
    const std::vector<std::string> &args) const
{
    if (args.size() != 1)
        return {BAD_PARAMETER_RESPONSE};
    return formatMapContent();
}

std::vector<std::string> GuiHandler::handleTeamNames(
    const std::vector<std::string> &args) const
{
    if (args.size() != 1)
        return {BAD_PARAMETER_RESPONSE};
    return formatTeamNames();
}

std::string GuiHandler::handlePlayerPosition(
    const std::vector<std::string> &args) const
{
    std::size_t playerId = 0;

    if (args.size() != 2 || !parsePlayerId(args[1], playerId))
        return BAD_PARAMETER_RESPONSE;
    return formatPlayerPosition(playerId);
}

std::string GuiHandler::handlePlayerLevel(
    const std::vector<std::string> &args) const
{
    std::size_t playerId = 0;

    if (args.size() != 2 || !parsePlayerId(args[1], playerId))
        return BAD_PARAMETER_RESPONSE;
    return formatPlayerLevel(playerId);
}

std::string GuiHandler::handlePlayerInventory(
    const std::vector<std::string> &args) const
{
    std::size_t playerId = 0;

    if (args.size() != 2 || !parsePlayerId(args[1], playerId))
        return BAD_PARAMETER_RESPONSE;
    return formatPlayerInventory(playerId);
}

std::string GuiHandler::handleTimeUnit(const std::vector<std::string> &args) const
{
    if (args.size() != 1)
        return BAD_PARAMETER_RESPONSE;
    return formatTimeUnit();
}

std::string GuiHandler::handleTimeUnitModification(
    const std::vector<std::string> &args)
{
    int frequency = 0;

    if (args.size() != 2 || !parsePositiveNumber(args[1], frequency)
        || frequency <= 0 || frequency > MAX_FREQUENCY)
        return BAD_PARAMETER_RESPONSE;
    _worldData.freq = frequency;
    return "sst " + std::to_string(_worldData.freq);
}

bool GuiHandler::isPositionValid(int x, int y) const
{
    return y >= 0 && y < _worldData.size.y
        && x >= 0 && x < _worldData.size.x;
}
