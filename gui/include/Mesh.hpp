#pragma once

#include <GL/glew.h>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <string>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <fstream>
#include <sstream>
#include <iostream>

#include "Shader.hpp"

struct Vertex {
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 TexCoords;

    glm::ivec4 BoneIds;
    glm::vec4  Weights;
};

struct Texture {
    unsigned int id;
    std::string type;
};  

class Mesh {
    public:
        std::vector<Vertex>       vertices;
        std::vector<unsigned int> indices; // ces 3 attributs devraient etre private a mon avis
        std::vector<Texture>      textures;

        Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures);
        ~Mesh();
        Mesh(const Mesh&) = delete;
        Mesh& operator=(const Mesh&) = delete;
        Mesh(Mesh&& other) noexcept;
        Mesh& operator=(Mesh&& other) noexcept;
        void Draw(Shader &shader);

    private:
        GLuint _VAO = 0;
        GLuint _VBO = 0;
        GLuint _EBO = 0;

        void setupMesh();
        void release();
};  
