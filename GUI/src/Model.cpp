#include "Model.hpp"

Model::Model(const std::string& filename)
{
    Assimp::Importer importer;

    importer.ReadFile(filename, aiProcess_Triangulate);

    const aiScene* scene = importer.GetScene();

    if (!scene || !scene->mRootNode || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) {
        std::cerr << "ERROR : Failed to load " << filename << '\n';
        return;
    }
}