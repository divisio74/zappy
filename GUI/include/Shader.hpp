#pragma once

#include <GL/glew.h>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <string>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <fstream>
#include <sstream>
#include <iostream>

class Shader {
    public:
        Shader(const std::string& vertPath, const std::string& fragPath);
        ~Shader();
        Shader(const Shader&) = delete;
        Shader& operator=(const Shader&) = delete;
        void use();
        void setMat4(const std::string& name, const glm::mat4& mat);
        void setInt(const std::string& name, int value);
        void setVec3(const std::string& name, const glm::vec3& vec);
    private:
        GLuint _id = 0;
        GLuint _compile(GLenum type, const std::string& path);
};
