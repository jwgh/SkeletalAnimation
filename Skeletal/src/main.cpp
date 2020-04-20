#include <memory>
#include <iostream>
#include <bitset>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>

#include "Model.h"
#include "Camera.h"

std::bitset<512> keyboard_status{ 0 };

int width { 800 };
int height{ 600 };

bool firstMouse{true};
bool mouse_free{true};
float lastX{width / 2.0f};
float lastY{height / 2.0f};


enum KEY{
    E = GLFW_KEY_E,
    S = GLFW_KEY_S,
    D = GLFW_KEY_D,
    F = GLFW_KEY_F,
    Q = GLFW_KEY_Q,
    A = GLFW_KEY_A,
    W = GLFW_KEY_W,
    R = GLFW_KEY_R,
    LALT = GLFW_KEY_LEFT_ALT,
    RALT = GLFW_KEY_RIGHT_ALT,
    ESC = GLFW_KEY_ESCAPE,

    RELEASE = GLFW_RELEASE
};

GLFWwindow* window;
std::shared_ptr<Model> model;
std::shared_ptr<Camera> camera;
std::shared_ptr<Shader> shader;
bool update_projection { true };

void key_callback(GLFWwindow* window, int key, int, int action, int) {
    switch(key){
        case E: keyboard_status[KEY::E] = action!=RELEASE; break;
        case S: keyboard_status[KEY::S] = action!=RELEASE; break;
        case D: keyboard_status[KEY::D] = action!=RELEASE; break;
        case F: keyboard_status[KEY::F] = action!=RELEASE; break;
        case Q: keyboard_status[KEY::Q] = action!=RELEASE; break;
        case A: keyboard_status[KEY::A] = action!=RELEASE; break;
        case W: keyboard_status[KEY::W] = action!=RELEASE; break;
        case R: keyboard_status[KEY::R] = action!=RELEASE; break;
        case ESC: keyboard_status[KEY::ESC] = action!=RELEASE; break;
        case LALT: keyboard_status[KEY::LALT] = action!=RELEASE; break;
        case RALT: keyboard_status[KEY::RALT] = action!=RELEASE; break;
        default: break;
    }
}

void framebuffer_size_callback(GLFWwindow* window, int w, int h) {
    glfwGetFramebufferSize(window, &width, &height);
    camera->update_ratio(width, height);
    update_projection = true;
    glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos){
    if(firstMouse){
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }
    if(!mouse_free) {
        float xoffset = xpos - lastX;
        float yoffset = lastY - ypos;
        lastX = xpos;
        lastY = ypos;

        float sensitivity{0.1f};
        xoffset *= sensitivity;
        yoffset *= sensitivity;

        camera->processMouseMovement(xoffset, yoffset, true);
    }
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset){
    camera->processMouseScroll(yoffset);
}

void init() {
    if(!glfwInit()){
        std::cout << "Unable to load GLFW" << std::endl;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    
    window = glfwCreateWindow(width, height, "Skeletal Animation", nullptr, nullptr);
    glfwMakeContextCurrent(window);
    if(!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)){
        std::cout << "Unable to load GLAD" << std::endl;
        return;
    }

    glfwSetKeyCallback(window, key_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    model = std::make_shared<Model>("../Resources/models/ninja/Run.fbx");
    camera = std::make_shared<Camera>(glm::vec3{0.0f, 80.0f, 365.0f}, width, height);
    shader = std::make_shared<Shader>("../Resources/shaders/skeletal.vert", "../Resources/shaders/simple.frag");
}

void process_input(double dt){
    static const double SPEED{ 28.0 };
    if(keyboard_status[KEY::ESC]){
        glfwSetWindowShouldClose(window, true);
    }
    if(keyboard_status[KEY::E]){
        camera->processKeyboard(Camera::Movement::FORWARD, dt * SPEED);
    }
    if(keyboard_status[KEY::S]){
        camera->processKeyboard(Camera::Movement::LEFT, dt * SPEED);
    }
    if(keyboard_status[KEY::D]){
        camera->processKeyboard(Camera::Movement::BACKWARD, dt * SPEED);
    }
    if(keyboard_status[KEY::F]){
        camera->processKeyboard(Camera::Movement::RIGHT, dt * SPEED);
    }
    if(keyboard_status[KEY::LALT] || keyboard_status[KEY::RALT]){
        mouse_free = !mouse_free;
        auto cursor_status = mouse_free ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED;
        glfwSetInputMode(window, GLFW_CURSOR, cursor_status);
    }
}

int main(int argc, char* argv[]) {
    init();
    double last_time{ 0.0 };
    double current_time { 0.0 };
    double dt{ 0.0 };
    while (!glfwWindowShouldClose(window)) {
        current_time = glfwGetTime();
        dt = current_time - last_time;
        last_time = current_time;
        
        glfwPollEvents();
        process_input(dt);
        
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shader->use();
        shader->set_uniform_m4("u_V", camera->get_view_matrix());
        if(update_projection){
            shader->set_uniform_m4("u_P", camera->get_proj_matrix());
            update_projection = false;
        }

        model->draw(0, *shader, current_time);
        //model->draw(*shader);
        
        glfwSwapBuffers(window);
    }
    return 0;
}