#include "app.hpp"

App::App()
{
    _renderer.init();
}

void App::run()
{
    while (_renderer.getWindow().isOpen()) {
        while (const std::optional eventOptional = _renderer.getWindow().pollEvent()) {
            if (eventOptional->is<sf::Event::Closed>())
                _renderer.getWindow().close();
        }
        _renderer.getWindow().display();
    }
}