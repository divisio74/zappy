#pragma once

#include <string>
#include <vector>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <bits/stdc++.h>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <optional>

#include "Mesh.hpp"
#include "Shader.hpp"

struct BoneInfo {
    glm::mat4 offsetMatrix;
    glm::mat4 finalMatrix;
};

class Model 
{
    public:
        Model(
            const std::string& path, 
            glm::vec3 position  = glm::vec3(0.f, 0.f, 0.f),
            glm::vec3 scale     = glm::vec3(1.f),
            glm::vec3 rotation  = glm::vec3(0.f),
            glm::vec3 offset    = glm::vec3(0.f)
        );

        void Draw(Shader &shader);
        void updateAnimation(float deltaTime, const std::optional<std::string>& animName = std::nullopt);
        void moveTo(glm::vec3 position);

    private:
        const aiScene*              _scene = nullptr;
        Assimp::Importer            _importer;
        std::vector<Mesh>           _meshes;
        std::string                 _directory;
        glm::vec3                   _position;
        glm::vec3                   _scale;
        glm::vec3                   _rotation;
        glm::vec3                   _offset;
        std::map<std::string, int>  _boneMap;
        std::vector<BoneInfo>       _bones;
        int                         _boneCount = 0;
        float                       _animTime  = 0.f;
        int                         _currentAnim = 0;

        void loadModel(const std::string& path);
        void processNode(aiNode *node, const aiScene *scene);
        Mesh processMesh(aiMesh *mesh, const aiScene *scene);
        void processNodeAnimation(aiNode* node, const glm::mat4& parentTransform);
        void setAnim(const std::string& animName);
        glm::vec3 interpolatePosition(aiNodeAnim *channel);
        glm::quat interpolateRotation(aiNodeAnim *channel);
        glm::vec3 interpolateScale(aiNodeAnim *channel);
        glm::mat4 aiMat4ToGlm(const aiMatrix4x4& m);
        static std::string toLower(const std::string& value);
        std::vector<Texture> loadMaterialTextures(aiMaterial *mat, aiTextureType type, std::string typeName, const aiScene *scene);
};
