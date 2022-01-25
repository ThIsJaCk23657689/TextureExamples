#include <SDL.h>
#include <SDL_mixer.h>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>
#include <memory>
#include <vector>

#include "MatrixStack.hpp"
#include "Shader.hpp"
#include "Texture.hpp"
#include "Camera.hpp"

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

std::unique_ptr<Shader> my_shader = nullptr;
std::unique_ptr<MatrixStack> model = nullptr;
std::unique_ptr<Camera> my_camera = nullptr;

static int keyFrameRate = 15.0;
float current_time = 0.0f;
float delta_time = 0.0f;
float last_time = 0.0f;

int main(int argc, char **argv) {

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) {
        std::cout << "SDL_Init Error: " << SDL_GetError() << std::endl;
        return -1;
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
#ifdef __APPLE__
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
#endif
#ifdef __linux__
    SDL_SetHint(SDL_HINT_VIDEO_X11_NET_WM_BYPASS_COMPOSITOR, "0");
#endif

    // 抗鋸齒
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);

    const auto flags = SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI;
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

    my_shader = std::make_unique<Shader>("assets/shaders/default.vert", "assets/shaders/default.frag");

    // 建立 Model Matrix Stack
    model = std::make_unique<MatrixStack>();

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    my_camera = std::make_unique<Camera>(glm::vec3(0.0f, 0.0f, 20.0f), glm::vec3(0.0f, 8.0f, 0.0f), true);
    my_camera->FollowTarget = false;

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

    std::vector<std::unique_ptr<Texture>> rickroll;
    for (int i = 0; i < 28; ++i) {
        rickroll.emplace_back(std::make_unique<Texture>("assets/textures/rickroll/rickroll (" + std::to_string(i + 1) + ").png"));
    }

    std::unique_ptr<Texture> my_background = std::make_unique<Texture>("assets/textures/background.png");

    int mix_flags = MIX_INIT_MP3;
    int initted = Mix_Init(flags);
    if(initted & flags != flags) {
        printf("Mix_Init: Failed to init required mp3 support!\n");
        printf("Mix_Init: %s\n", Mix_GetError());
        exit(1);
    }

    if(Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 1024) < 0) {
        printf("Mix_OpenAudio failed \n", Mix_GetError());
        Mix_CloseAudio();
        exit(1);
    }
    Mix_Music *music = Mix_LoadMUS("assets/sounds/bg.mp3");
    Mix_PlayMusic(music, 1);

    bool isDone = false;

    while (!isDone) {
        // 計算每 frame 的變化時間
        current_time = static_cast<float>(SDL_GetTicks()) / 1000.0f;
        delta_time = current_time - last_time;
        last_time = current_time;

        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    isDone = true;
                    break;
                case SDL_KEYDOWN: {
                    switch (event.key.keysym.sym) {
                        case SDLK_TAB:
                            my_camera->ToggleMouseControl();
                            break;
                        case SDLK_q:
                            if (KMOD_CTRL & event.key.keysym.mod) {
                                isDone = true;
                            }
                            break;
                    }
                } break;
            }
        }

        my_camera->ProcessKeyboard();
        my_camera->ProcessMouseMovement();
        my_camera->Update(delta_time);

        // 設定 View 以及 Projection Matrix
        glm::mat4 view = my_camera->View();
        glm::mat4 projection = my_camera->Projection();

        my_shader->Use();
        my_shader->SetMat4("view", view);
        my_shader->SetMat4("projection", projection);
        my_shader->SetInt("ourTexture", 0);

        glViewport(0, 0, window_width, window_height);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glActiveTexture(GL_TEXTURE0);
        glBindVertexArray(vao);

        model->Push();
        model->Save(glm::translate(model->Top(), glm::vec3((glm::sin(current_time * 3.4333f) * 2) - 1, 0.0, 0.0f)));
        model->Save(glm::translate(model->Top(), glm::vec3(0.0, 8.0, 0.0f)));
        model->Save(glm::scale(model->Top(), glm::vec3(16.0, 16.0, 0.0f)));
        rickroll[static_cast<int>(current_time * keyFrameRate) % 28]->Bind();
        my_shader->SetMat4("model", model->Top());
        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
        model->Pop();

        model->Push();
        my_background->Bind();
        model->Save(glm::translate(model->Top(), glm::vec3(0.0, 10.0, -5.0f)));
        model->Save(glm::scale(model->Top(), glm::vec3(20.0, 20.0, 0.0f)));
        my_shader->SetMat4("model", model->Top());
        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
        model->Pop();

        model->Push();
        my_background->Bind();
        model->Save(glm::rotate(model->Top(), glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f)));
        model->Save(glm::scale(model->Top(), glm::vec3(100.0, 100.0, 0.0f)));
        my_shader->SetMat4("model", model->Top());
        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
        model->Pop();

        SDL_GL_SwapWindow(window);
    }

    Mix_FreeMusic(music);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}