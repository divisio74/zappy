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
 * socket, then assign stable ids to each accepted client connection.
 *
 * Typical flow:
 * - addServerSocket(port) creates the listening server socket on port.
 * - pollSockets() updates whether a new client can be accepted and returns the
 *   ids of client sockets that have data waiting.
 * - hasPendingConnection() tells whether acceptConnection(id) can be called.
 * - acceptConnection(id) accepts the new client and stores it behind id.
 * - getLostConnections() returns and removes sockets whose client disconnected.
 * - read(id) returns complete protocol commands from that socket.
 * - write(id, command) sends one protocol command to that socket.
 *
 * Commands passed to write() and returned by read() do not include the "\n"
 * delimiter. The delimiter is handled by the socket I/O layer.
 *
 * The caller does not need to keep file descriptors or vector indexes. Client
 * ids are the public handles, which makes it easy to keep matching business
 * objects in another container.
 */
class SocketRegistry {
public:
    static constexpr std::size_t MaxSockets = 1000;

    SocketRegistry();

    /**
     * Creates the listening server socket on a TCP port.
     *
     * Only one server socket can listen in a registry. Connected clients are
     * added with acceptConnection(id).
     */
    void addServerSocket(std::uint16_t port);

    /**
     * Returns true if pollSockets() detected a client waiting to be accepted.
     */
    bool hasPendingConnection() const;

    /**
     * Accepts a new client connection and stores it behind id.
     *
     * The id must be unique while the client is registered.
     */
    void acceptConnection(std::size_t id);

    /**
     * Removes the socket identified by id.
     *
     * After this call, the id is no longer valid for read, write, or polling.
     */
    void removeSocket(std::size_t id);

    /**
     * Returns the ids of client sockets that can be read immediately.
     *
     * This call is non-blocking. It also updates hasPendingConnection() when
     * the listening socket can accept a new client. Sockets with a closed or
     * invalid connection are kept aside and can be collected with
     * getLostConnections().
     */
    std::vector<std::size_t> pollSockets();

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

    /**
     * Returns the number of registered client sockets.
     */
    std::size_t size() const;

private:
    struct SocketEntry {
        std::size_t id;
        ClientSocket socket;
    };

    std::optional<ServerSocket> _serverSocket;
    std::vector<SocketEntry> _sockets;
    std::array<int, MaxSockets> _fds;
    std::unordered_map<std::size_t, std::size_t> _indexById;
    std::unordered_map<int, std::size_t> _idByFd;
    std::vector<std::size_t> _lostConnections;
    bool _hasPendingConnection;

    std::size_t getIndex(std::size_t id) const;
    void markLostConnection(std::size_t id);
    void refreshFds();
};
