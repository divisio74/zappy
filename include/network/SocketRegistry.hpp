#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

#include "network/ServerSocket.hpp"

/**
 * Manages several one-to-one server sockets behind stable ids.
 *
 * SocketRegistry is a small network manager: callers choose the socket id and
 * the TCP port, then use that same id for every operation.
 *
 * Typical flow:
 * - addSocket(id, port) creates the server socket on port.
 * - the child process/client receives port and connects to it.
 * - acceptConnection(id) accepts that connection.
 * - pollSockets() returns the ids of sockets that have data waiting.
 * - getLostConnections() returns and removes sockets whose client disconnected.
 * - read(id) returns complete protocol commands from that socket.
 * - write(id, command) sends one protocol command to that socket.
 *
 * Commands passed to write() and returned by read() do not include the "\n"
 * delimiter. The delimiter is handled by the socket I/O layer.
 *
 * The caller does not need to keep file descriptors or vector indexes. The id
 * is the public handle, which makes it easy to keep a matching business object
 * such as KitchenState in another container.
 */
class SocketRegistry {
public:
    static constexpr std::size_t MaxSockets = 1000;

    SocketRegistry();

    /**
     * Creates a server socket for an externally chosen id and TCP port.
     *
     * The id must be unique while the socket is registered. The port is usually
     * given to the process that will create the matching ClientSocket.
     */
    void addSocket(std::size_t id, std::uint16_t port);

    /**
     * Accepts the client connection for the socket identified by id.
     *
     * Call this after the matching client process has been started and is ready
     * to connect.
     */
    void acceptConnection(std::size_t id);

    /**
     * Removes the socket identified by id.
     *
     * After this call, the id is no longer valid for read, write, or polling.
     */
    void removeSocket(std::size_t id);

    /**
     * Returns the ids of sockets that can be read immediately.
     *
     * This call is non-blocking. It returns an empty vector if no socket has data
     * waiting. Sockets with a closed or invalid connection are kept aside and can
     * be collected with getLostConnections().
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
     * Returns the number of registered server sockets.
     */
    std::size_t size() const;

private:
    struct SocketEntry {
        std::size_t id;
        ServerSocket socket;
    };

    std::vector<SocketEntry> _sockets;
    std::array<int, MaxSockets> _fds;
    std::unordered_map<std::size_t, std::size_t> _indexById;
    std::unordered_map<int, std::size_t> _idByFd;
    std::vector<std::size_t> _lostConnections;

    std::size_t getIndex(std::size_t id) const;
    void markLostConnection(std::size_t id);
    void refreshFds();
};
