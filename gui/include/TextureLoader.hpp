#pragma once

#include <GL/glew.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <string>

class TextureLoader {
    public:
        static GLuint load(const std::string& path, const std::string& directory = "gui/textures", const aiScene *scene = nullptr);
};
