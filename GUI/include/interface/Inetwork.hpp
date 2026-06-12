#pragma once
#include <string>
#include <vector>

class INetwork {
public:
    virtual ~INetwork() = default;
    virtual bool connect(const std::string &host, int port) = 0;
    virtual bool isConnected() const = 0;
    virtual std::vector<std::string> receive() = 0;
    virtual void send(const std::string &cmd) = 0;
};
