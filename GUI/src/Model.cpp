#include "Model.hpp"
#include "TextureLoader.hpp"

// Doc : https://learnopengl.com/Model-Loading/Model


Model::Model(const std::string &path, glm::vec3 scale, glm::vec3 rotation, glm::vec3 offset)
{
    loadModel(path);
    _scale = scale;
    _rotation = rotation;
    _offset = offset;
}

void Model::Draw(Shader &shader, glm::vec3 position)
{
    glm::mat4 model = glm::mat4(1.f);
    model = glm::translate(model, position + _offset);
    model = glm::rotate(model, glm::radians(_rotation.x), glm::vec3(1.f, 0.f, 0.f));
    model = glm::rotate(model, glm::radians(_rotation.y), glm::vec3(0.f, 1.f, 0.f));
    model = glm::rotate(model, glm::radians(_rotation.z), glm::vec3(0.f, 0.f, 1.f));
    model = glm::scale(model, _scale);
    shader.setMat4("model", model);
    for (auto& mesh : meshes)
        mesh.Draw(shader);
}

void Model::loadModel(const std::string &path)
{
    Assimp::Importer import;
    const aiScene *scene = import.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);	

    if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) 
    {
        std::cout << "ERROR::ASSIMP::" << import.GetErrorString() << std::endl;
        return;
    }
    directory = path.substr(0, path.find_last_of('/'));

    processNode(scene->mRootNode, scene);
}

void Model::processNode(aiNode *node, const aiScene *scene)
{
    for(unsigned int i = 0; i < node->mNumMeshes; i++) {
        aiMesh *mesh = scene->mMeshes[node->mMeshes[i]]; 
        meshes.push_back(processMesh(mesh, scene));			
    }

    for(unsigned int i = 0; i < node->mNumChildren; i++)
        processNode(node->mChildren[i], scene);
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

        vertices.push_back(vertex);
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
        texture.id   = TextureLoader::load(str.C_Str(), directory, scene);
        texture.type = typeName;
        texture.path = str;
        textures.push_back(texture);
    }
    return textures;
} 