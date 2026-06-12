#include "../include/NetworkProtocol.hpp"
#include "../include/GameState.hpp"
#include <iostream>
#include <thread>
#include <chrono>

int main(int argc, char **argv)
{
    if (argc != 3) {
        std::cerr << "Usage: ./zappy_gui -p port\n";
        return 84;
    }

    int port = std::stoi(argv[2]);
    GameState state;
    NetworkProtocol network;

    if (!network.connect("127.0.0.1", port))
        return 84;

    while (network.isConnected()) {
        network.poll(state);
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    return 0;
}
