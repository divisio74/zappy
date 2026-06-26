#include "core/Core.hpp"
#include "parser/Parser.hpp"

#include <exception>
#include <iostream>

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
        return 84;
    }
}
