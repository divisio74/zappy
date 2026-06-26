#include "../include/NetworkProtocol.hpp"
#include "../include/GameState.hpp"
#include "Gui.hpp"
#include <filesystem>
#include <iostream>
#include <thread>
#include <chrono>

int main(int argc, char **argv)
{
    if (argc != 3) {
        std::cerr << "Usage: ./zappy_gui -p port\n";
        return 84;
    }

    if (std::filesystem::exists("GUI"))
        std::filesystem::current_path("GUI");

    int port = std::stoi(argv[2]);
    Gui gui;

    if (!gui.getNetwork().connect("127.0.0.1", port))
        return 84;

    while (gui.getNetwork().isConnected() && gui.getRenderer().getWindow().isOpen()) {
        gui.getNetwork().poll(gui.getState());
        gui.run();
    }

    return 0;
}
