#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include "network/ClientSocket.hpp"
#include "network/ServerSocket.hpp"

/**
 * Manages one listening server socket and several connected clients.
 *
 * SocketRegistry is a small network manager: callers create one TCP listening
 * socket, then gives stable technical ids to each accepted client connection.
 *
 * Typical flow:
 * - addServerSocket(port) creates the listening server socket on port.
 * - pollSockets() updates whether a new client can be accepted and returns the
 *   ids of typed client sockets that have data waiting.
 * - getPendingIdentificationSockets() returns untyped sockets that are ready
 *   to identify themselves.
 * - hasPendingConnection() tells whether acceptConnection() can be called.
 * - acceptConnection() accepts the new client and returns its socket id.
 * - getLostConnections() returns and removes sockets whose client disconnected.
 * - read(id) returns complete protocol commands from that socket.
 * - write(id, command) sends one protocol command to that socket.
 *
 * Commands passed to write() and returned by read() do not include the "\n"
 * delimiter. The delimiter is handled by the socket I/O layer.
 *
 * The caller does not need to keep file descriptors or vector indexes. Client
 * ids are technical handles. A business id can be attached later, once the
 * client type is known.
 */
class SocketRegistry {
public:
    enum class SocketState {
        Undefined,
        Gui,
        Ai,
    };

    static constexpr std::size_t MaxSockets = 1000;

    SocketRegistry();

    /**
     * Creates the listening server socket on a TCP port.
     *
     * Only one server socket can listen in a registry. Connected clients are
     * added with acceptConnection().
     */
    void addServerSocket(std::uint16_t port);

    /**
     * Returns true if pollSockets() detected a client waiting to be accepted.
     */
    bool hasPendingConnection() const;

    /**
     * Accepts a new client connection and returns its technical socket id.
     */
    std::size_t acceptConnection();

    /**
     * Removes the socket identified by id.
     *
     * After this call, the id is no longer valid for read, write, or polling.
     */
    void removeSocket(std::size_t id);

    /**
     * Returns the ids of typed client sockets that can be read immediately.
     *
     * This call is non-blocking. It also updates hasPendingConnection() when
     * the listening socket can accept a new client. Untyped readable sockets
     * are kept aside and can be collected with
     * getPendingIdentificationSockets(). Sockets with a closed or invalid
     * connection are kept aside and can be collected with getLostConnections().
     */
    std::vector<std::size_t> pollSockets();

    /**
     * Returns the ids of untyped sockets that are ready to identify themselves.
     *
     * The returned sockets stay registered. This only consumes the pending
     * identification notification produced by the last pollSockets() call.
     */
    std::vector<std::size_t> getPendingIdentificationSockets();

    /**
     * Returns the ids of sockets whose connection was lost since the last call.
     *
     * The returned sockets are removed from the registry before this function
     * returns, so their ids are no longer valid for read, write, or polling.
     */
    std::vector<std::size_t> getLostConnections();

    /**
     * Reads complete protocol commands from the socket identified by id.
     *
     * Returned commands do not include the trailing "\n". If the received
     * data does not contain a complete command yet, this returns an empty
     * vector and keeps the partial data buffered for the next read().
     *
     * Several commands may be returned at once if several complete messages
     * were received on the same socket.
     */
    std::vector<std::string> read(std::size_t id, std::size_t size = 4096);

    /**
     * Writes one protocol command to the socket identified by id.
     *
     * The command should be passed without "\n"; the delimiter is appended
     * automatically by the socket I/O layer.
     */
    void write(std::size_t id, const std::string &message);

    SocketState getSocketState(std::size_t id) const;
    std::optional<std::size_t> getBusinessId(std::size_t id) const;
    void setSocketIdentity(std::size_t id, SocketState state,
                           std::size_t businessId);

    /**
     * Returns the number of registered client sockets.
     */
    std::size_t size() const;

private:
    struct SocketEntry {
        std::size_t id;
        ClientSocket socket;
        SocketState state;
        std::optional<std::size_t> businessId;
    };

    std::optional<ServerSocket> _serverSocket;
    std::vector<SocketEntry> _sockets;
    std::array<int, MaxSockets> _fds;
    std::unordered_map<std::size_t, std::size_t> _indexById;
    std::unordered_map<int, std::size_t> _idByFd;
    std::vector<std::size_t> _lostConnections;
    std::vector<std::size_t> _pendingIdentifications;
    std::size_t _nextSocketId;
    bool _hasPendingConnection;

    std::size_t getIndex(std::size_t id) const;
    void markLostConnection(std::size_t id);
    void refreshFds();
};
