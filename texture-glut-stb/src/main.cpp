#include <imgui.h>
#include <imgui_impl_glut.h>
#include <imgui_impl_opengl2.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "stb_image.h"

#include <iostream>
#include <string>
#include <memory>
#include <vector>
#include <chrono>

#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/freeglut.h>
#endif

static int window_width = 1280, window_height = 720;
static bool show_demo_window = true;
static float fovy = 45.0f;
static std::vector<bool> key_state = std::vector<bool>(512, false);

// Time unit
long last_time = 0;
float delta_time = 0.0;

unsigned int RickRollTexture;

struct Camera {
    glm::vec3 position;
    glm::vec3 velocity;
    glm::vec3 acceleration;

    glm::vec3 target;
    glm::vec3 worldUp;

    glm::vec3 right;
    glm::vec3 up;
    glm::vec3 front;
    float pitch;
    float yaw;
    float speed;

    Camera() :
        position(glm::vec3(0.0f, 0.0f, 3.0f)),
        velocity(glm::vec3(0.0f, 0.0f, 0.0f)),
        acceleration(glm::vec3(0.0f, 0.0f, 0.0f)),
        target(glm::vec3(0.0f, 0.0f, 0.0f)),
        worldUp(glm::vec3(0.0f, 1.0f, 0.0f)),
        right(glm::vec3(1.0f, 0.0f, 0.0f)),
        up(glm::vec3(0.0f, 1.0f, 0.0f)),
        front(glm::vec3(0.0f, 0.0f, -1.0f)),
        pitch(0.0f),
        yaw(0.0f),
        speed(0.1f) {}

    void update() {
        velocity += acceleration;
        acceleration = glm::vec3(0.0f);

        position += velocity;
        velocity *= 0.95f;
    }

    void view() {
        target = position + front;
        gluLookAt(
            position[0], position[1], position[2], target[0], target[1], target[2], worldUp[0], worldUp[1], worldUp[2]);
    }

    void updateAxes() {
        glm::mat4 rotate_matrix = glm::mat4(1.0f);

        rotate_matrix = glm::rotate(rotate_matrix, glm::radians(-yaw), glm::vec3(0.0, 1.0, 0.0));
        rotate_matrix = glm::rotate(rotate_matrix, glm::radians(pitch), glm::vec3(1.0, 0.0, 0.0));

        // camera always face to negative-z, and the z-axis of the camera is (0, 0,
        // 1), which is the back direction of the camara.
        glm::vec4 z_direction = rotate_matrix * glm::vec4(0.0, 0.0, 1.0, 1.0);
        front = glm::normalize(glm::vec3(z_direction.x, z_direction.y, z_direction.z));
        right = glm::normalize(glm::cross(worldUp, front));
        up = glm::normalize(glm::cross(front, right));
    }
};

std::unique_ptr<Camera> my_camera;

static unsigned int loadTexture(const std::string& file) {
    unsigned int texture_id;

    glEnable(GL_TEXTURE_2D);
    glGenTextures(1, &texture_id);
    glBindTexture(GL_TEXTURE_2D, texture_id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(true);
    unsigned char *image = stbi_load(file.c_str(), &width, &height, &nrChannels, 0);
    if (image) {
        GLenum internal_format(-1);
        GLenum format(-1);

        switch (nrChannels) {
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
    } else {
        std::cout << "Failed to load texture" << std::endl;
        exit(-42069);
    }
    stbi_image_free(image);
    return texture_id;
}

static void genTextures() {
    RickRollTexture = loadTexture("assets/textures/rickroll.png");
}



static void normalKeys(unsigned char key, int x, int y) {
    // ESC for escape the application.
    if (key == 27) {
        exit(0);
    }

    if (key == 'W' || key == 'w') {
        my_camera->acceleration.x += -glm::cos(glm::radians(my_camera->yaw + 90)) * my_camera->speed;
        my_camera->acceleration.z += -glm::sin(glm::radians(my_camera->yaw + 90)) * my_camera->speed;
    }

    if (key == 'S' || key == 's') {
        my_camera->acceleration.x += glm::cos(glm::radians(my_camera->yaw + 90)) * my_camera->speed;
        my_camera->acceleration.z += glm::sin(glm::radians(my_camera->yaw + 90)) * my_camera->speed;
    }

    if (key == 'A' || key == 'a') {
        my_camera->acceleration.x += -glm::cos(glm::radians(my_camera->yaw)) * my_camera->speed;
        my_camera->acceleration.z += -glm::sin(glm::radians(my_camera->yaw)) * my_camera->speed;
    }

    if (key == 'D' || key == 'd') {
        my_camera->acceleration.x += glm::cos(glm::radians(my_camera->yaw)) * my_camera->speed;
        my_camera->acceleration.z += glm::sin(glm::radians(my_camera->yaw)) * my_camera->speed;
    }

    if (key == ' ') {
        my_camera->acceleration.y += my_camera->speed;
    }

    ImGui_ImplGLUT_KeyboardFunc(key, x, y);
}

static void specialKeys(int key, int x, int y) {
    switch (key) {
        case GLUT_KEY_SHIFT_L:
            my_camera->acceleration.y -= my_camera->speed;
            break;
    }
}

static void reshape(int w, int h) {
    window_width = w > 1 ? w : 1;
    window_height = h > 1 ? h : 1;
    glViewport(0, 0, window_width, window_height);

    float aspect = (float)w / (float)h;
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    if (w <= h) {
        gluPerspective(fovy, aspect, 0.1f, 100.0f);
    } else {
        gluPerspective(fovy, aspect, 0.1f, 100.0f);
    }
}

static void drawImgui() {
    if (show_demo_window) {
        ImGui::ShowDemoWindow(&show_demo_window);
    }

    static int counter = 0;
    ImGui::Begin("Computer Graphics");
    ImGui::Text("Press ESC to exit");
    ImGui::Checkbox("Demo Window", &show_demo_window);
    ImGui::SliderFloat("fovy", &fovy, 1.0f, 179.0f);

    ImGui::Text(
        "Camera Position: (%.2f, %.2f, %.2f)", my_camera->position.x, my_camera->position.y, my_camera->position.z);

    // ImGui::ColorEdit3("background color", &clear_color[0]);
    if (ImGui::Button("Button")) {
        counter++;
    }
    ImGui::SameLine();
    ImGui::Text("counter = %d", counter);
    ImGui::Text(
        "Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
    ImGui::End();
}

static void drawTriangle() {
    // glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glBindTexture(GL_TEXTURE_2D, RickRollTexture);
    glColor3f(1.0f, 1.0f, 1.0f);
    glBegin(GL_POLYGON);
    glTexCoord2f(0.0, 0.0);
    glVertex3f(-0.5f, -0.5f, 0.0f);
    glTexCoord2f(1.0, 0.0);
    glVertex3f(0.5f, -0.5f, 0.0f);
    glTexCoord2f(1.0, 1.0);
    glVertex3f(0.5f, 0.5f, 0.0f);
    glTexCoord2f(0.0, 1.0);
    glVertex3f(-0.5f, 0.5f, 0.0f);
    glEnd();
}

static void drawFloor() {
    glColor3f(0.825f, 0.014f, 0.145f);
    glBegin(GL_QUADS);
    glVertex3f(-100.0f, 0.0f, -100.0f);
    glVertex3f(-100.0f, 0.0f, 100.0f);
    glVertex3f(100.0f, 0.0f, 100.0f);
    glVertex3f(100.0f, 0.0f, -100.0f);
    glEnd();
}

static void display() {
    // Start the Dear ImGui frame
    ImGui_ImplOpenGL2_NewFrame();
    ImGui_ImplGLUT_NewFrame();

    drawImgui();

    // Rendering Imgui
    ImGui::Render();
    ImGuiIO& io = ImGui::GetIO();
    reshape((GLsizei)io.DisplaySize.x, (GLsizei)io.DisplaySize.y);

    my_camera->update();

    // Clear screen
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    my_camera->view();

    // Draw ground
    drawFloor();

    // Draw a colorful triangle
    drawTriangle();

    // glutSolidSphere(1.0f, 30, 30);

    // Or u can draw a Utah teapot.
    // glutSolidTeapot(0.5);

    ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());

    glutSwapBuffers();
    glutPostRedisplay();
}

void idle(void) {
    using namespace std::chrono;
    auto time = duration_cast<nanoseconds>(system_clock::now().time_since_epoch()).count();
    delta_time = (time - last_time) / 1000000000.0f;

    // current_keyframe_idx = static_cast<unsigned int>((current_keyframe_idx + 1 * delta_time) / 89.0f);
}

int main(int argc, char** argv) {
    // GLUT window initialization
    glutInit(&argc, argv);
#ifdef __FREEGLUT_EXT_H__
    glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS);
#endif
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_MULTISAMPLE | GLUT_DEPTH);
    glutInitWindowPosition(80, 80);
    glutInitWindowSize(window_width, window_height);
    glutCreateWindow("Texture Example: GLUT with stb_image");

    // Register callbacks
    glutDisplayFunc(display);

    // Show information
    std::cout << "GPU Vendor: " << glGetString(GL_VENDOR) << std::endl;
    std::cout << "GPU Renderer: " << glGetString(GL_RENDERER) << std::endl;
    std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void)io;
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer backends
    ImGui_ImplGLUT_Init();
    ImGui_ImplGLUT_InstallFuncs();
    ImGui_ImplOpenGL2_Init();

    // This code must after the call to ImGui_ImplGLUT_InstallFuncs().
    glutIdleFunc(idle);
    glutKeyboardFunc(normalKeys);
    glutSpecialFunc(specialKeys);

    // Init Camera
    my_camera = std::make_unique<Camera>();

    // Gen Texture
    genTextures();

    glutMainLoop();

    // Cleanup
    ImGui_ImplOpenGL2_Shutdown();
    ImGui_ImplGLUT_Shutdown();
    ImGui::DestroyContext();
    return 0;
}