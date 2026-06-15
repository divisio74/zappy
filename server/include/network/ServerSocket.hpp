#pragma once

#include <cstddef>
#include <cstdint>
#include <netinet/in.h>
#include <vector>

#include "network/ClientSocket.hpp"
#include "network/IOSocket.hpp"

class ServerSocket {
public:
    enum class ConnectionType {
        Reciprocal
    };

    explicit ServerSocket(std::uint16_t port, ConnectionType type = ConnectionType::Reciprocal);
    ~ServerSocket();

    ServerSocket(const ServerSocket &) = delete;
    ServerSocket &operator=(const ServerSocket &) = delete;
    ServerSocket(ServerSocket &&other);
    ServerSocket &operator=(ServerSocket &&other);

    void acceptConnection();
    std::vector<std::string> read(std::size_t size = 4096);
    void write(const std::string &message) const;
    int getFd() const;

private:
    static int toNativeType(ConnectionType type);
    static sockaddr_in makeAddress(std::uint16_t port);
    void closeSocket();

    IOSocket _ioSocket;
    bool _connected;
};
