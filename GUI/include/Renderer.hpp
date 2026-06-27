#pragma once

#include <GL/glew.h>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <memory>
#include <string>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <fstream>
#include <sstream>
#include <iostream>
#include <tuple>

#include "TextureLoader.hpp"
#include "Camera.hpp"
#include "Tile.hpp"
#include "Shader.hpp"
#include "Mesh.hpp"
#include "GameState.hpp"
#include "NetworkProtocol.hpp"


class Renderer {
    public:
        Renderer();
        ~Renderer() = default;

        const sf::VideoMode&    getVideoMode() const;
        const sf::RenderWindow& getWindow() const;
        sf::RenderWindow&       getWindow();
        Camera&                 getCamera();

        void init();
        void render(GameState& state, float deltaTime);
        void renderMap(std::vector<std::vector<Tile>>& map);
        void renderTile(Tile& tile, float x, float z);
        std::unique_ptr<Mesh> initTile();
        void initPlayersModels(GameState& state);
        void uptPlayerModel(int playerID, std::shared_ptr<Model> &playerModel, GameState& state);
        glm::vec3 getTileCenter (const Tile& tile);
        glm::vec3 getTileRandPos(const Tile& tile);
        std::shared_ptr<Model> getModel(const std::string path, glm::vec3 scale = glm::vec3(1.f));

        std::map<int, std::shared_ptr<Model>> _playersModels; //TODO mettre en private et adapté partout dans le code

    private:
        sf::VideoMode              _videoMode;
        sf::ContextSettings        _settings;
        sf::RenderWindow           _window;
        sf::Font                   _font;
        Camera                     _camera;
        std::unique_ptr<Shader>    _shader;
        std::unique_ptr<Mesh>      _tileMesh;
        std::vector<GLuint>        _groundTextures;
        std::map<int, std::string> _playerAnimations;
        std::map<int, float>       _playerAnimationTimers;
        std::map<std::string, std::shared_ptr<Model>> _modelsCache;
        void renderHUD(const GameState &state);
};
