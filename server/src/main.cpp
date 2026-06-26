#include "core/Core.hpp"
#include "parser/Parser.hpp"

#include <exception>
#include <iostream>
#include <string>

namespace {
const char *USAGE = "USAGE: ./zappy_server -p port -x width -y height -n name1 ... -c clientsNb -f freq";
}

int main(int argc, char **argv)
{
    Parser parser;

    try {
        ServerConfig config = parser.parse(argc, argv);
        Core core(config);

        core.runServer();
        return 0;
    } catch (const std::exception &error) {
        std::cerr << error.what() << std::endl;
        if (std::string(error.what()).find("USAGE:") == std::string::npos)
            std::cerr << USAGE << std::endl;
        return 84;
    }
}
