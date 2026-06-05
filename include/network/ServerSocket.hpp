#pragma once

#include <cstddef>
#include <string>
#include <sys/un.h>
#include <vector>

#include "network/ClientSocket.hpp"
#include "network/IOSocket.hpp"

class ServerSocket {
public:
    enum class ConnectionType {
        Reciprocal
    };

    explicit ServerSocket(const std::string &path, ConnectionType type = ConnectionType::Reciprocal);
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
    static sockaddr_un makeAddress(const std::string &path);
    void closeSocket();

    IOSocket _ioSocket;
    std::string _path;
    bool _connected;
};
