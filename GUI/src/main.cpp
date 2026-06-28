#include "../include/NetworkProtocol.hpp"
#include "../include/GameState.hpp"
#include "Gui.hpp"
#include <filesystem>
#include <iostream>
#include <thread>
#include <chrono>

int main(int argc, char **argv)
{
    int port = -1;
    std::string host = "127.0.0.1";

    for (int i = 1; i < argc - 1; i++) {
        std::string arg = argv[i];

        if (arg == "-p")
            port = std::stoi(argv[i + 1]);
        else if (arg == "-h")
            host = argv[i + 1];
    }

    if (port == -1) {
        std::cerr << "Usage: ./zappy_gui -p port -h machine\n";
        return 84;
    }

    if (std::filesystem::exists("GUI"))
        std::filesystem::current_path("GUI");

    Gui gui;

    if (!gui.getNetwork().connect(host, port))
        return 84;

    while (gui.getNetwork().isConnected() && gui.getRenderer().getWindow().isOpen()) {
        gui.getNetwork().poll(gui.getState());
        gui.run();
    }

    return 0;
}
