#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <netinet/in.h>
#include <vector>

#include "network/IOSocket.hpp"

class ClientSocket {
public:
    explicit ClientSocket(std::uint16_t port);
    ClientSocket(const std::string &host, std::uint16_t port);
    ~ClientSocket();

    ClientSocket(const ClientSocket &) = delete;
    ClientSocket &operator=(const ClientSocket &) = delete;
    ClientSocket(ClientSocket &&other);
    ClientSocket &operator=(ClientSocket &&other);

    std::vector<std::string> read(std::size_t size = 4096);
    void write(const std::string &message) const;
    int getFd() const;

private:
    explicit ClientSocket(int fd);
    static sockaddr_in makeAddress(const std::string &host, std::uint16_t port);
    void closeSocket();

    IOSocket _ioSocket;

    friend class ServerSocket;
};