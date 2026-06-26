#pragma once

#include <functional>
#include <queue>
#include <string>
#include <unordered_map>
#include <vector>

#include "core/handlers/GuiEvent.hpp"
#include "core/world/WorldData.hpp"

struct message {
    size_t playerId;
    std::string message;
};

class AiHandler {
public:
    AiHandler(WorldData& world, std::function<void(const GuiEvent&)> queueGuiEvent);
    ~AiHandler() = default;

    void parseCommand(size_t playerId, std::vector<std::string>& commands);
    void run();
    std::vector<message> getMessages();
    void addMessage(size_t id, std::string message);

private:
    void enqueueCommand(Player& player, const std::string& command);
    void executeCommand(size_t playerId, const std::string& command);

    Player& findPlayer(size_t playerId) const;
    Vector2 normalizePos(Vector2) const;
    bool canIncantation(const Player& player) const;
    int countPlayersAtLevel(const Vector2 &coord, int level) const;

    void queueGuiEvent(const GuiEvent &event);

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
    std::function<void(const GuiEvent&)> _queueGuiEvent;
    std::queue<message> _messageQueue;

    static const std::unordered_map<std::string, int> ACTION_COSTS;
};
