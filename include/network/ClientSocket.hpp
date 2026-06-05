#pragma once

#include <cstddef>
#include <string>
#include <sys/un.h>
#include <vector>

#include "network/IOSocket.hpp"

class ClientSocket {
public:
    explicit ClientSocket(const std::string &path);
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
    static sockaddr_un makeAddress(const std::string &path);
    void closeSocket();

    IOSocket _ioSocket;

    friend class ServerSocket;
};
