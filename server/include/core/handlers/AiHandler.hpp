#pragma once

#include "core/world/WorldData.hpp"
#include <unordered_map>

struct message {
    size_t playerId;
    std::string message;
};

class AiHandler {
public:
    AiHandler(WorldData& world) : _world(world) {};
    ~AiHandler() = default;

    void parseCommand(size_t playerId, std::vector<std::string>& commands);
    void run();

private:
    void enqueueCommand(Player& player, const std::string& command);
    void executeCommand(size_t playerId, const std::string& command);

    Player& findPlayer(size_t playerId) const;
    Vector2 normalizePos(Vector2) const;
    const Tile& getTile(const Vector2& pos) const;

    void addMessage(size_t id, std::string message);

    void execForward(Player& player);
    void execRight(Player& player);
    void execLeft(Player& player);
    void execLook(Player& player);
    void execInventory(Player& player);
    void execConnectNbr(Player& player);
    void execFork(Player& player);
    void execEject(Player& player);
    void execIncantation(Player& player);
    void execBroadcast(Player& player, const std::string& text);
    void execTake(Player& player, const std::string& object);
    void execSet(Player& player, const std::string& object);

    WorldData&  _world;
    std::queue<message> _messageQueue;

    static const std::unordered_map<std::string, int> ACTION_COSTS;
};
