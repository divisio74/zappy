#pragma once

#define HEIGHT  10
#define WIDTH   10

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

#include "TextureLoader.hpp"
#include "Camera.hpp"
#include "Tile.hpp"
#include "Shader.hpp"
#include "Mesh.hpp"
#include "Model.hpp"

class Renderer {
    public:
        Renderer();
        ~Renderer() = default;

        void init();
        GLuint TextureFromFile(const char* path, const std::string& directory);
        void render();
        void renderMap(uint width, uint height);        // temporaire apres ca prendra les infos de gamestate
        void renderTile(float x, float z);
        std::unique_ptr<Mesh> initTile();

        const sf::VideoMode& getVideoMode() const;
        const sf::Window&    getWindow   () const;
        sf::Window&          getWindow   ();
        Camera&              getCamera   ();

    private:
        sf::VideoMode                   _videoMode;
        sf::ContextSettings             _settings;
        sf::Window                      _window;
        Camera                          _camera;
        std::unique_ptr<Shader>         _shader;
        std::unique_ptr<Mesh>           _tileMesh;
        std::unique_ptr<Model>          _playerModel;
};