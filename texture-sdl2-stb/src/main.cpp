#include "Shader.hpp"

#include <glad/glad.h>
#include <SDL.h>
#include "stb_image.h"

#include <iostream>
#include <vector>

static unsigned int window_width = 800;
static unsigned int window_height = 600;

static std::vector<float> vertices = {
    // Position             // Texture
    -0.5f, -0.5f, 0.0f,     0.0f, 0.0f,
     0.5f, -0.5f, 0.0f,     1.0f, 0.0f,
     0.5f,  0.5f, 0.0f,     1.0f, 1.0f,
    -0.5f,  0.5f, 0.0f,     0.0f, 1.0f,
};

static std::vector<unsigned int> indices = {
    0, 1, 2,
    0, 2, 3,
};

int main(int argc, char **argv) {

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::cout << "SDL_Init Error: " << SDL_GetError() << std::endl;
        return -1;
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);

    auto const flags = SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI;
    SDL_Window *window = SDL_CreateWindow("Texture Example: SDL2 with stb_image", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, window_width, window_height, flags);
    if (window == nullptr) {
        std::cout << "SDL_CreateWindow Error: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    SDL_SetWindowMinimumSize(window, 400, 300);
    auto glContext = SDL_GL_CreateContext(window);
    SDL_GL_MakeCurrent(window, glContext);
    SDL_GL_SetSwapInterval(1);

    gladLoadGLLoader(SDL_GL_GetProcAddress);

    std::cout << "OpenGL Version:        " << glGetString(GL_VERSION) << "\n"
              << "GLSL Version:          " << glGetString(GL_SHADING_LANGUAGE_VERSION) << "\n"
              << "Renderer:              " << glGetString(GL_RENDERER) << "\n"
              << "Vendor:                " << glGetString(GL_VENDOR) << std::endl;

    using namespace shader;

    GLuint program = createProgram();
    attachShader(program, GL_VERTEX_SHADER, "assets/shaders/default.vert");
    attachShader(program, GL_FRAGMENT_SHADER, "assets/shaders/default.frag");
    linkProgram(program);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    GLuint vao;
    GLuint vbo;
    GLuint ebo;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), reinterpret_cast<const void *>(0));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), reinterpret_cast<const void *>(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);


    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(true);
    unsigned char *image = stbi_load("assets/textures/rickroll.png", &width, &height, &nrChannels, 0);
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

    bool isDone = false;

    while (!isDone) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    isDone = true;
                    break;
                case SDL_KEYDOWN: {
                    switch (event.key.keysym.sym) {
                        case SDLK_q:
                            if (KMOD_CTRL & event.key.keysym.mod) {
                                isDone = true;
                            }
                            break;
                    }
                } break;
            }
        }

        glUniform1i(glGetUniformLocation(program, "ourTexture"), 0);

        glViewport(0, 0, window_width, window_height);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glUseProgram(program);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);
        glBindVertexArray(vao);
        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);

        SDL_GL_SwapWindow(window);
    }

    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}