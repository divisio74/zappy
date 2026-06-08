#include "app.hpp"

App::App()
{
    _videoMode.size.x  = 1200;
    _videoMode.size.y = 800;
    _renderWindow.create(_videoMode, "Zappy");
}

void App::run()
{
    while (_renderWindow.isOpen()) {
        while (const std::optional eventOptional = _renderWindow.pollEvent()) {
            if (eventOptional->is<sf::Event::Closed>())
                _renderWindow.close();
        }
        _renderWindow.clear();
        _renderWindow.display();
    }
}