#pragma once
#include "./interface/Inetwork.hpp"
#include "network/ClientSocket.hpp"
#include <memory>

class NetworkBase : public INetwork {

public:
    NetworkBase();
    ~NetworkBase() = default;
    bool connect(const std::string &host, int port) override;
    bool isConnected() const override { return _connected; }
    std::vector<std::string> receive() override;
    void send(const std::string &cmd) override;
private:
    std::unique_ptr<ClientSocket> _socket;
    bool _connected;
};
