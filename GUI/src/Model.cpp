#include "Model.hpp"
#include "TextureLoader.hpp"
#include <set>

// Doc : https://learnopengl.com/Model-Loading/Model

Model::Model(const std::string &path, glm::vec3 position, glm::vec3 scale, glm::vec3 rotation, glm::vec3 offset)
{
    loadModel(path);
    _position = position;
    _scale = scale;
    _rotation = rotation;
    _offset = offset;
}

Model::~Model()
{
    std::set<GLuint> textures;

    for (const Mesh &mesh : _meshes) {
        for (const Texture &texture : mesh.textures) {
            if (texture.id != 0)
                textures.insert(texture.id);
        }
    }
    for (GLuint texture : textures)
        glDeleteTextures(1, &texture);
}

void Model::Draw(Shader &shader)
{
    glm::mat4 model = glm::mat4(1.f);
    model = glm::translate(model, _position + _offset);
    model = glm::rotate(model, glm::radians(_rotation.y), glm::vec3(0.f, 1.f, 0.f));
    model = glm::rotate(model, glm::radians(_rotation.x), glm::vec3(1.f, 0.f, 0.f));
    model = glm::rotate(model, glm::radians(_rotation.z), glm::vec3(0.f, 0.f, 1.f));
    model = glm::scale(model, _scale);
    shader.setMat4("model", model);

    for (int i = 0; i < (int)_bones.size(); i++) {
        std::string name = "boneMatrices[" + std::to_string(i) + "]";
        shader.setMat4(name, _bones[i].finalMatrix);
    }

    for (auto& mesh : _meshes)
        mesh.Draw(shader);
}

void Model::Draw(Shader &shader, glm::vec3 position)
{
    glm::mat4 model = glm::translate(glm::mat4(1.f), position + _offset);
    model = glm::rotate(model, glm::radians(_rotation.x), glm::vec3(1.f, 0.f, 0.f));
    model = glm::rotate(model, glm::radians(_rotation.y), glm::vec3(0.f, 1.f, 0.f));
    model = glm::rotate(model, glm::radians(_rotation.z), glm::vec3(0.f, 0.f, 1.f));
    model = glm::scale(model, _scale);
    shader.setMat4("model", model);

    for (auto& mesh : _meshes)
        mesh.Draw(shader);
}

void Model::loadModel(const std::string &path)
{
    _scene = _importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);

    if(!_scene || _scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !_scene->mRootNode) 
    {
        std::cout << "ERROR::ASSIMP::" << _importer.GetErrorString() << std::endl;
        return;
    }
    _directory = path.substr(0, path.find_last_of('/'));
    _globalInverseTransform = glm::inverse(aiMat4ToGlm(_scene->mRootNode->mTransformation));

    processNode(_scene->mRootNode, _scene);
    processNodeAnimation(_scene->mRootNode, glm::mat4(1.f), nullptr);
}

void Model::processNode(aiNode *node, const aiScene *scene)
{
    for(unsigned int i = 0; i < node->mNumMeshes; i++) {
        aiMesh *mesh = scene->mMeshes[node->mMeshes[i]]; 
        _meshes.push_back(processMesh(mesh, scene));			
    }

    for(unsigned int i = 0; i < node->mNumChildren; i++)
        processNode(node->mChildren[i], scene);
}

glm::mat4 Model::aiMat4ToGlm(const aiMatrix4x4& m)
{
    return glm::transpose(glm::mat4(
        m.a1, m.a2, m.a3, m.a4,
        m.b1, m.b2, m.b3, m.b4,
        m.c1, m.c2, m.c3, m.c4,
        m.d1, m.d2, m.d3, m.d4
    ));
}

Mesh Model::processMesh(aiMesh *mesh, const aiScene *scene)
{
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::vector<Texture> textures;

    for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
        Vertex vertex;

        vertex.Position = { mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z };
        vertex.Normal = mesh->mNormals ? glm::vec3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z) : glm::vec3(0.f);
        vertex.TexCoords = mesh->mTextureCoords[0] ? glm::vec2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y) : glm::vec2(0.f);
        vertex.BoneIds = glm::ivec4(0);
        vertex.Weights = glm::vec4(0.f);

        vertices.push_back(vertex);
    }

    for (unsigned int i = 0; i < mesh->mNumBones; i++) {
    aiBone* bone = mesh->mBones[i];
    std::string name = bone->mName.C_Str();

    if (_boneMap.find(name) == _boneMap.end()) {
        _boneMap[name] = _boneCount;
        BoneInfo info;
        info.offsetMatrix = aiMat4ToGlm(bone->mOffsetMatrix);
        info.finalMatrix = glm::mat4(1.f);
        _bones.push_back(info);
        _boneCount++;
    }

    int boneIndex = _boneMap[name];

    for (unsigned int j = 0; j < bone->mNumWeights; j++) {
        int   vertexId = bone->mWeights[j].mVertexId;
        float weight   = bone->mWeights[j].mWeight;

        for (int k = 0; k < 4; k++) {
            if (vertices[vertexId].Weights[k] == 0.f) {
                vertices[vertexId].BoneIds[k] = boneIndex;
                vertices[vertexId].Weights[k] = weight;
                break;
            }
        }
    }
}

    for(unsigned int i = 0; i < mesh->mNumFaces; i++) {
        aiFace face = mesh->mFaces[i];
        for(unsigned int j = 0; j < face.mNumIndices; j++)
            indices.push_back(face.mIndices[j]);
    }
    if(mesh->mMaterialIndex >= 0) {
        aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];
    std::vector<Texture> diffuseMaps  = loadMaterialTextures(material, aiTextureType_DIFFUSE,  "texture_diffuse",  scene);
    textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
    std::vector<Texture> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular", scene);
    textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
    }  

    return Mesh(vertices, indices, textures);
}

// Version optimisée : https://learnopengl.com/Model-Loading/Model
std::vector<Texture> Model::loadMaterialTextures(aiMaterial *mat, aiTextureType type, std::string typeName, const aiScene *scene)
{
    std::vector<Texture> textures;
    for (unsigned int i = 0; i < mat->GetTextureCount(type); i++) {
        aiString str;
        mat->GetTexture(type, i, &str);
        Texture texture;
        texture.id   = TextureLoader::load(str.C_Str(), _directory, scene);
        texture.type = typeName;
        textures.push_back(texture);
    }
    return textures;
}

glm::vec3 Model::interpolatePosition(aiNodeAnim* channel)
{
    if (channel->mNumPositionKeys == 1)
        return glm::vec3(channel->mPositionKeys[0].mValue.x,
                         channel->mPositionKeys[0].mValue.y,
                         channel->mPositionKeys[0].mValue.z);

    unsigned int idx = 0;
    for (unsigned int i = 0; i < channel->mNumPositionKeys - 1; i++) {
        if (_animTime < channel->mPositionKeys[i + 1].mTime) { idx = i; break; }
    }

    float t1 = channel->mPositionKeys[idx].mTime;
    float t2 = channel->mPositionKeys[idx + 1].mTime;
    float factor = (_animTime - t1) / (t2 - t1);  // 0.0 → 1.0

    aiVector3D& p1 = channel->mPositionKeys[idx].mValue;
    aiVector3D& p2 = channel->mPositionKeys[idx + 1].mValue;

    return glm::mix(glm::vec3(p1.x, p1.y, p1.z),glm::vec3(p2.x, p2.y, p2.z), factor);
}

glm::quat Model::interpolateRotation(aiNodeAnim* channel)
{
    if (channel->mNumRotationKeys == 1) {
        auto& q = channel->mRotationKeys[0].mValue;
        return glm::quat(q.w, q.x, q.y, q.z);
    }

    unsigned int idx = 0;
    for (unsigned int i = 0; i < channel->mNumRotationKeys - 1; i++) {
        if (_animTime < channel->mRotationKeys[i + 1].mTime) {
            idx = i;
            break;
        }
    }

    float t1 = channel->mRotationKeys[idx].mTime;
    float t2 = channel->mRotationKeys[idx + 1].mTime;
    float factor = (_animTime - t1) / (t2 - t1);

    auto& q1 = channel->mRotationKeys[idx].mValue;
    auto& q2 = channel->mRotationKeys[idx + 1].mValue;

    return glm::slerp(glm::quat(q1.w, q1.x, q1.y, q1.z), glm::quat(q2.w, q2.x, q2.y, q2.z), factor);
}

glm::vec3 Model::interpolateScale(aiNodeAnim* channel)
{
    if (channel->mNumScalingKeys == 1)
        return glm::vec3(channel->mScalingKeys[0].mValue.x,
                         channel->mScalingKeys[0].mValue.y,
                         channel->mScalingKeys[0].mValue.z);

    unsigned int idx = 0;
    for (unsigned int i = 0; i < channel->mNumScalingKeys - 1; i++) {
        if (_animTime < channel->mScalingKeys[i + 1].mTime) {
            idx = i;
            break;
        }
    }

    float t1 = channel->mScalingKeys[idx].mTime;
    float t2 = channel->mScalingKeys[idx + 1].mTime;
    float factor = (_animTime - t1) / (t2 - t1);

    aiVector3D& s1 = channel->mScalingKeys[idx].mValue;
    aiVector3D& s2 = channel->mScalingKeys[idx + 1].mValue;

    return glm::mix(glm::vec3(s1.x, s1.y, s1.z), glm::vec3(s2.x, s2.y, s2.z), factor);
}

void Model::updateAnimation(float deltaTime, const std::optional<std::string>& animName)
{
    if (!_scene || _scene->mNumAnimations == 0)
        return;

    if (animName)
        setAnim(*animName);
    if (_currentAnim < 0 || static_cast<unsigned int>(_currentAnim) >= _scene->mNumAnimations)
        _currentAnim = 0;

    aiAnimation* anim = _scene->mAnimations[_currentAnim];
    float tps = anim->mTicksPerSecond != 0 ? anim->mTicksPerSecond : 25.f;
    _animTime += deltaTime * tps;
    _animTime  = fmod(_animTime, anim->mDuration);

    processNodeAnimation(_scene->mRootNode, glm::mat4(1.f), anim);
}

void Model::moveTo(glm::vec3 newPosition, glm::vec3 newRotation)
{
    _position = newPosition;
    _rotation = newRotation;
}

void Model::processNodeAnimation(aiNode* node, const glm::mat4& parentTransform, aiAnimation *anim)
{
    std::string nodeName = node->mName.C_Str();

    glm::mat4 nodeTransform = aiMat4ToGlm(node->mTransformation);

    aiNodeAnim* channel = nullptr;
    if (anim != nullptr) {
        for (unsigned int i = 0; i < anim->mNumChannels; i++) {
            if (strcmp(anim->mChannels[i]->mNodeName.C_Str(), nodeName.c_str()) == 0) {
                channel = anim->mChannels[i];
                break;
            }
        }
    }

    if (channel) {
        glm::vec3 position = interpolatePosition(channel);
        glm::quat rotation = interpolateRotation(channel);
        glm::vec3 scale    = interpolateScale(channel);

        nodeTransform = glm::translate(glm::mat4(1.f), position)
                      * glm::mat4_cast(rotation)
                      * glm::scale(glm::mat4(1.f), scale);
    }

    glm::mat4 globalTransform = parentTransform * nodeTransform;

    if (_boneMap.count(nodeName)) {
        int idx = _boneMap[nodeName];
        _bones[idx].finalMatrix = _globalInverseTransform * globalTransform * _bones[idx].offsetMatrix;
    }

    for (unsigned int i = 0; i < node->mNumChildren; i++)
        processNodeAnimation(node->mChildren[i], globalTransform, anim);
}

std::string Model::toLower(const std::string& value)
{
    std::string result = value;
    std::transform(result.begin(), result.end(), result.begin(),
        [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return result;
}

void Model::setAnim(const std::string &animName)
{
    if (!_scene || _scene->mNumAnimations == 0)
        return;

    const std::string targetName = toLower(animName);

    for (unsigned int i = 0; i < _scene->mNumAnimations; ++i) {

        aiAnimation* anim = _scene->mAnimations[i];
        std::string currName = toLower(anim->mName.C_Str());

        if (currName.find(targetName) != std::string::npos) {
            _currentAnim = i;
            return;
        }
    }
}
