#include "Texture.hpp"

Texture::Texture(const std::string &filename) : id(0) {
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(true);
    unsigned char *image = stbi_load(filename.c_str(), &width, &height, &nrChannels, 0);
    if (image) {
        GLenum internal_format(-1);
        GLenum format(-1);

        switch (nrChannels) {
            case 1:
                internal_format = GL_R8;
                format = GL_RED;
                break;
            case 3:
                internal_format = GL_RGB8;
                format = GL_RGB;
                break;
            case 4:
                internal_format = GL_RGBA8;
                format = GL_RGBA;
                break;
            default:
                std::cout << "The Images File format is not supported yet!"  << std::endl;
                break;
        }

        glTexImage2D(GL_TEXTURE_2D, 0, internal_format, width, height, 0, format, GL_UNSIGNED_BYTE, image);
        glGenerateMipmap(GL_TEXTURE_2D);
    } else {
        std::cout << "Failed to load texture" << std::endl;
        exit(-42069);
    }
    stbi_image_free(image);
}

Texture::~Texture() {
    glDeleteTextures(1, &id);
}

void Texture::Bind() {
    glBindTexture(GL_TEXTURE_2D, id);
}