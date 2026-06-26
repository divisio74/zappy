#include "TextureLoader.hpp"

GLuint TextureLoader::load(const std::string& path, const std::string& directory, const aiScene *scene)
{
    GLuint texId;
    glGenTextures(1, &texId);
    glBindTexture(GL_TEXTURE_2D, texId);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    sf::Image image;

    if (scene) {
        const aiTexture* embeddedTex = scene->GetEmbeddedTexture(path.c_str());
        if (embeddedTex) {
            image.loadFromMemory(embeddedTex->pcData, embeddedTex->mWidth);
        }
    } else {
        std::string fullPath = directory.empty() ? path : directory + "/" + path;
        image.loadFromFile(fullPath);
    }

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
                 image.getSize().x, image.getSize().y,
                 0, GL_RGBA, GL_UNSIGNED_BYTE, image.getPixelsPtr());
    glGenerateMipmap(GL_TEXTURE_2D);

    return texId;
}