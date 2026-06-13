#pragma once

#include <string>
#include <vector>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <fstream>
#include <sstream>
#include <iostream>

#include "Mesh.hpp"
#include "Shader.hpp"

class Model 
{
    public:
        Model(const std::string& path, glm::vec3 scale = glm::vec3(1.f), glm::vec3 rotation  = glm::vec3(0.f), glm::vec3 offset = glm::vec3(0.f));

        void Draw(Shader &shader, glm::vec3 position);
    private:
        std::vector<Mesh> meshes;
        std::string directory;
        glm::vec3 _scale;
        glm::vec3 _rotation;
        glm::vec3 _offset;

        void loadModel(const std::string& path);
        void processNode(aiNode *node, const aiScene *scene);
        Mesh processMesh(aiMesh *mesh, const aiScene *scene);
        std::vector<Texture> loadMaterialTextures(aiMaterial *mat, aiTextureType type, std::string typeName, const aiScene *scene);
};