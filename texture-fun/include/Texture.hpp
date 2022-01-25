#pragma once

#include <glad/glad.h>
#include "stb_image.h"
#include <iostream>
#include <string>

struct Texture {
    unsigned int id;
    Texture(const std::string& filename);
    ~Texture();
    void Bind();
};