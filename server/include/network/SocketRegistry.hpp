#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include "network/ClientSocket.hpp"
#include "network/ServerSocket.hpp"

class SocketRegistry {
public:
    enum class SocketState {
        Unknown,
        Ai,
        Gui
    };

    static constexpr std::size_t MaxSockets = 1000;
    static constexpr std::size_t ListenerSocketId = 0;

    SocketRegistry();
    SocketRegistry(const std::string &host, std::uint16_t port,
        std::size_t maxClients);

    void removeSocket(std::size_t socketId);
    std::vector<std::size_t> pollSockets(int timeoutMs);
    std::vector<std::size_t> getNewConnections();

    void assignBusinessId(std::size_t socketId, std::size_t businessId);
    bool hasBusinessId(std::size_t socketId) const;
    std::optional<std::size_t> getBusinessId(std::size_t socketId) const;
    std::optional<std::size_t> getSocketIdByBusinessId(std::size_t businessId) const;
    void setSocketState(std::size_t socketId, SocketState state);
    SocketState getSocketState(std::size_t socketId) const;
    std::vector<std::size_t> getSocketIdsByState(SocketState state) const;

    std::vector<std::size_t> getLostConnections();
    std::vector<std::string> read(std::size_t socketId, std::size_t size = 4096);
    void write(std::size_t socketId, const std::string &message);
    std::size_t size() const;

private:
    struct SocketEntry {
        std::size_t socketId;
        SocketState state;
        std::optional<std::size_t> businessId;
        ClientSocket socket;
    };

    ServerSocket _listener;
    std::size_t _maxClients;
    std::size_t _nextSocketId;
    std::vector<SocketEntry> _sockets;
    std::unordered_map<std::size_t, std::size_t> _indexById;
    std::unordered_map<std::size_t, std::size_t> _socketIdByBusinessId;
    std::unordered_map<int, std::size_t> _idByFd;
    std::vector<std::size_t> _newConnections;
    std::vector<std::size_t> _lostConnections;

    std::size_t getIndex(std::size_t socketId) const;
    void acceptNewConnection();
    void markLostConnection(std::size_t socketId);
    void refreshIndexes();
};
