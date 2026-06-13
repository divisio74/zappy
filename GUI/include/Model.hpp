#pragma once

#include "Renderer.hpp"
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

class Model {
    public:
        Model(const std::string& filename);
        ~Model() = default;

    private:
        std::vector<float> vertices;
        std::vector<unsigned int> indices;
};