#pragma once

#include <GL/glew.h>
#include <SFML/Graphics.hpp>
#include <glm/glm.hpp>
#include <memory>
#include <string>
#include <vector>

#include "Shader.hpp"

class Cubemap {
    public:
        void init();
        void release();
        void render(const glm::mat4 &view, const glm::mat4 &projection);

    private:
        unsigned int loadTexture(const std::vector<std::string> &faces);
        void initMesh();
        void deleteTexture(GLuint &texture);

        std::unique_ptr<Shader> _shader;
        unsigned int _vao = 0;
        unsigned int _vbo = 0;
        unsigned int _texture = 0;
};
