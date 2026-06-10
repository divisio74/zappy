#pragma once

#include <GL/glew.h>
#include <SFML/Window.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <fstream>
#include <sstream>
#include <iostream>

#include "Camera.hpp"

class Renderer {
    public:
        Renderer();
        ~Renderer() = default;

        void init();
        void render();

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

        GLuint _compileShader(GLenum type, const std::string& path);
        GLuint _createProgram(const std::string& vertPath, const std::string& fragPath);
};