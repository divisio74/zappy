#pragma once

#define HEIGHT  5
#define WIDTH   5

#include <GL/glew.h>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <fstream>
#include <sstream>
#include <iostream>

#include "Camera.hpp"
#include "Tile.hpp"

class Renderer {
    public:
        Renderer();
        ~Renderer() = default;

        void init(uint width, uint height);             // temporaire apres ca prendra les infos de gamestate
        void render();
        void renderMap(uint width, uint height);        // temporaire apres ca prendra les infos de gamestate
        void initTile(
            float offsetX,
            float offsetY, 
            float offsetZ
        );
        void renderTile(float x, float z);              // FAUT QUE JE METTE AVEC TILE

        const sf::VideoMode& getVideoMode() const;
        const sf::Window&    getWindow   () const;
        sf::Window&          getWindow   ();
        Camera&              getCamera   ();

    private:
        sf::VideoMode       _videoMode;
        sf::ContextSettings _settings;
        sf::Window          _window;
        Camera              _camera;
        GLuint              _shader;
        GLuint              _VAO;
        GLuint              _VBO;
        GLuint              _EBO;
        GLuint              _texture;

        GLuint _compileShader(GLenum type, const std::string& path);
        GLuint _createProgram(const std::string& vertPath, const std::string& fragPath);
};