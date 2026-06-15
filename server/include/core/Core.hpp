#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#include "core/handlers/AiHandler.hpp"
#include "core/handlers/GuiHandler.hpp"
#include "core/world/WorldData.hpp"
#include "network/SocketRegistry.hpp"

class Core {
public:
    Core();
    Core(std::uint16_t port, std::size_t width, std::size_t height,
        const std::vector<std::string> &teamNames, std::size_t clientsNb,
        std::size_t frequency);

    WorldData &getWorldData();
    const WorldData &getWorldData() const;
    SocketRegistry &getSocketRegistry();
    const SocketRegistry &getSocketRegistry() const;
    GuiHandler &getGuiHandler();
    AiHandler &getAiHandler();

private:
    WorldData _worldData;
    SocketRegistry _socketRegistry;
    GuiHandler _guiHandler;
    AiHandler _aiHandler;
};
