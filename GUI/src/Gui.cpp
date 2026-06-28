#include "Gui.hpp"

Gui::Gui()
{
    _renderer.init();
}

Gui::~Gui()
{
    _state.clear();
}

void Gui::run()
{
    while (const std::optional eventOptional = _renderer.getWindow().pollEvent()) {
        if (eventOptional->is<sf::Event::Closed>()) {
            _state.clear();
            _renderer.shutdown();
            return;
        }
        if (const auto *key = eventOptional->getIf<sf::Event::KeyPressed>()) {
            if (key->code == sf::Keyboard::Key::Escape) {
                _state.clear();
                _renderer.shutdown();
                return;
            }
            if (key->code == sf::Keyboard::Key::T)
                _renderer.toggleGroundOutline();
        }
        if (const auto *mouse = eventOptional->getIf<sf::Event::MouseButtonPressed>()) {
            if (mouse->button == sf::Mouse::Button::Left)
                _renderer.selectTileAt(_state, mouse->position);
        }
    }

    float deltaTime = _clock.restart().asSeconds();
    _renderer.getCamera().processInput(deltaTime);
    _renderer.getCamera().processMouse(_renderer.getWindow());

    glClearColor(0.1f, 0.1f, 0.15f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    _renderer.render(_state, deltaTime);
    _renderer.getWindow().display();
}

IRenderer &Gui::getRenderer()
{
    return _renderer;
}

GameState &Gui::getState()
{
    return _state;
}

NetworkProtocol &Gui::getNetwork()
{
    return _network;
}

// encapsuler libs graphiques
