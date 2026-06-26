#include "application/Application.hpp"

int Application::run(int argc, char **argv)
{
    ServerConfig config = _parser.parse(argc, argv);
    Core core(config);

    core.runServer();
    return 0;
}
