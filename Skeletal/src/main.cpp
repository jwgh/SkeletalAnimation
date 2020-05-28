#include <memory>
#include <iostream>
#include <bitset>
#include <random>
#include <chrono>
#include <thread>


#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>

#include "Model.h"
#include "Camera.h"
#include "Light.h"
#include "ParticleSystem.h"
#include "TextureManager.h"
#include "Player.h"
#include "Terrain.h"
#include "Sky.h"

std::bitset<512> keyboard_status{ 0 };
std::bitset<512> keyboard_status_prev{ 0 };

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
    T = GLFW_KEY_T,

    SPACE = GLFW_KEY_SPACE,
    LALT = GLFW_KEY_LEFT_ALT,
    RALT = GLFW_KEY_RIGHT_ALT,
    ESC = GLFW_KEY_ESCAPE,

    UP = GLFW_KEY_UP,
    DOWN = GLFW_KEY_DOWN,
    LEFT = GLFW_KEY_LEFT,
    RIGHT = GLFW_KEY_RIGHT,

    RELEASE = GLFW_RELEASE
};

GLFWwindow* window;
Player player;
std::shared_ptr<Model> run;
std::shared_ptr<Model> idle;
std::shared_ptr<Camera> camera;
std::shared_ptr<Shader> shader;
std::shared_ptr<Shader> particle_shader;
std::shared_ptr<Shader> skybox_shader;
std::shared_ptr<Shader> gbuffer_shader;
std::shared_ptr<Shader> deferred_shader;
std::shared_ptr<Shader> light_box;
LightPoint point_light;
LightDirectional sun;
bool update_projection { true };
GLuint smoke;
GLuint textures[3];


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
        case T: keyboard_status[KEY::T] = action!=RELEASE; break;
        case SPACE: keyboard_status[KEY::SPACE] = action!=RELEASE; break;
        case UP: keyboard_status[KEY::UP] = action!=RELEASE; break;
        case DOWN: keyboard_status[KEY::DOWN] = action!=RELEASE; break;
        case LEFT: keyboard_status[KEY::LEFT] = action!=RELEASE; break;
        case RIGHT: keyboard_status[KEY::RIGHT] = action!=RELEASE; break;
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
    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;
    if(!mouse_free) {
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
    glEnable(GL_PROGRAM_POINT_SIZE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    idle = std::make_shared<Model>("../Resources/models/ninja/Ninja Idle.fbx");
    run = std::make_shared<Model>("../Resources/models/ninja/Run.fbx");
    player.current_animation = idle.get();
    player.idle = idle.get();
    player.run = run.get();

    camera = std::make_shared<Camera>(glm::vec3{0.0f, 2.0f, 10.0f}, width, height);
    shader = std::make_shared<Shader>("../Resources/shaders/skeletal.vert", "../Resources/shaders/simple.frag");
    particle_shader = std::make_shared<Shader>("../Resources/shaders/particle.vert", "../Resources/shaders/particle.frag");
    skybox_shader = std::make_shared<Shader>("../Resources/shaders/sky.vert", "../Resources/shaders/sky.frag");

    gbuffer_shader = std::make_shared<Shader>("../Resources/shaders/gbuffer.vert", "../Resources/shaders/gbuffer.frag");
    deferred_shader = std::make_shared<Shader>("../Resources/shaders/deferred.vert", "../Resources/shaders/deferred.frag");
    light_box = std::make_shared<Shader>("../Resources/shaders/lightBox.vert", "../Resources/shaders/lightBox.frag");


    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 150");

    textures[0] = TextureManager::load_single_color_texture(128, 255, 255);
    textures[1] = TextureManager::load_single_color_texture(255, 128, 128);
    textures[2] = TextureManager::load_single_color_texture(128, 128, 255);
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

    player.current_animation = idle.get();
    if(keyboard_status[KEY::UP]){
        player.processMovement(Player::Movement::FORWARD, dt);
        player.current_animation = run.get();
    }
    if(keyboard_status[KEY::DOWN]){
        player.processMovement(Player::Movement::BACKWARD, dt);
        player.current_animation = run.get();
    }
    if(keyboard_status[KEY::LEFT]){
        //model->processMovement(Model::Movement::STRAFT_LEFT, dt);
        player.processMovement(Player::Movement::TURN_LEFT, dt);
        player.current_animation = run.get();
    }
    if(keyboard_status[KEY::RIGHT]){
        //model->processMovement(Model::Movement::STRAFE_RIGHT, dt);
        player.processMovement(Player::Movement::TURN_RIGHT, dt);
        player.current_animation = run.get();
    }

    if(keyboard_status[KEY::T]){
    }


    if((keyboard_status[KEY::LALT] || keyboard_status[KEY::RALT]) && !(keyboard_status_prev[KEY::LALT] || keyboard_status_prev[KEY::RALT])){
        mouse_free = !mouse_free;
        auto cursor_status = mouse_free ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED;
        glfwSetInputMode(window, GLFW_CURSOR, cursor_status);
    }
    if(keyboard_status[KEY::SPACE]&& !keyboard_status_prev[KEY::SPACE]){
    }

    keyboard_status_prev = keyboard_status;
}

bool test{ false };
float col[3];
bool deferred{ false };
void imgui_start_frame(){
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    static int counter = 0;

    ImGui::Begin("Hello, world!");

    ImGui::Text("Adv Graphics Project");
    ImGui::Checkbox("Imgui checkbox", &test);
    ImGui::Checkbox("Deferred Rendering", &deferred);

    ImGui::SliderFloat("x", &point_light.position.x, -100.0f, 100.0f);
    ImGui::SliderFloat("y", &point_light.position.y, -100.0f, 100.0f);
    ImGui::SliderFloat("z", &point_light.position.z, -100.0f, 100.0f);
    ImGui::ColorEdit3("clear color", (float*)&col);

    if (ImGui::Button("counter")){
        counter++;
    }
    ImGui::SameLine();
    ImGui::Text("counter = %d", counter);

    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
    ImGui::End();

    ImGui::Render();
}


void renderQuad();
void renderCube();


int main(int argc, char* argv[]) {
    init();
    double last_time{ 0.0 };
    double current_time { 0.0 };
    double dt{ 0.0 };

    std::shared_ptr<Shader> terrain_shader = std::make_shared<Shader>("../Resources/shaders/terrain.vert", "../Resources/shaders/terrain.frag");

    double PRE = glfwGetTime();
    Terrain terrain(0, 0); // 1, 1?
    std::cout << "Generation time: " << glfwGetTime() - PRE;


    ParticleSystem particle_system(particle_shader, 10000);

    smoke = TextureManager::load_texture_from_file("../Resources/textures/smoke.png");

    /**
     * INIT THE G_BUFFER
     * For Deferred rendering
     *
     */
    unsigned int gBuffer;
    glGenFramebuffers(1, &gBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
    unsigned int gPosition, gNormal, gAlbedoSpec;
    // position color buffer
    glGenTextures(1, &gPosition);
    glBindTexture(GL_TEXTURE_2D, gPosition);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width*2, height*2, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gPosition, 0);
    // normal color buffer
    glGenTextures(1, &gNormal);
    glBindTexture(GL_TEXTURE_2D, gNormal);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width*2, height*2, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gNormal, 0);
    // color + specular color buffer
    glGenTextures(1, &gAlbedoSpec);
    glBindTexture(GL_TEXTURE_2D, gAlbedoSpec);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width*2, height*2, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gAlbedoSpec, 0);
    // tell OpenGL which color attachments we'll use (of this framebuffer) for rendering
    unsigned int attachments[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
    glDrawBuffers(3, attachments);
    // create and attach depth buffer (renderbuffer)
    unsigned int rboDepth;
    glGenRenderbuffers(1, &rboDepth);
    glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width*2, height*2);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);
    // finally check if framebuffer is complete
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "Framebuffer not complete!" << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);




    // lighting info
    // -------------
    const unsigned int NR_LIGHTS = 32;
    std::vector<glm::vec3> lightPositions;
    std::vector<glm::vec3> lightColors;
    srand(1337);
    for (unsigned int i = 0; i < NR_LIGHTS; i++)
    {
        // calculate slightly random offsets
        float xPos = ((rand() % 100) / 100.0) * 120 - 60;
        float yPos = ((rand() % 100) / 100.0) * 36 - 12;
        float zPos = ((rand() % 100) / 100.0) * 120 - 60;
        lightPositions.emplace_back(glm::vec3(xPos, yPos, zPos));
        // also calculate random color
        float rColor = ((rand() % 100) / 200.0f) + 0.5; // between 0.5 and 1.0
        float gColor = ((rand() % 100) / 200.0f) + 0.5; // between 0.5 and 1.0
        float bColor = ((rand() % 100) / 200.0f) + 0.5; // between 0.5 and 1.0
        lightColors.emplace_back(glm::vec3(rColor, gColor, bColor));
    }



    std::vector<glm::vec3> objectPositions;
    objectPositions.emplace_back(glm::vec3(-30.0,  -5, -30.0));
    objectPositions.emplace_back(glm::vec3( 0.0,  -5, -30.0));
    objectPositions.emplace_back(glm::vec3( 30.0,  -5, -30.0));
    objectPositions.emplace_back(glm::vec3(-30.0,  -5,  0.0));
    objectPositions.emplace_back(glm::vec3( 0.0,  -5,  0.0));
    objectPositions.emplace_back(glm::vec3( 30.0,  -5,  0.0));
    objectPositions.emplace_back(glm::vec3(-30.0,  -5,  30.0));
    objectPositions.emplace_back(glm::vec3( 0.0,  -5,  30.0));
    objectPositions.emplace_back(glm::vec3( 30.0,  -5,  30.0));


    /**
    * DEFERRED RENDERING INIT END
    * MAIN LOOP START
    *
    */
    while (!glfwWindowShouldClose(window)) {
        current_time = glfwGetTime();
        dt = current_time - last_time;
        last_time = current_time;

        glfwPollEvents();
        process_input(dt);

        particle_system.update(dt);
        player.update(dt);
        imgui_start_frame();

        glClearColor(col[0], col[1], col[2], 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


        if(!deferred){


            terrain_shader->use();
            terrain_shader->set_uniform_v3("u_light.ambient", sun.ambient);
            terrain_shader->set_uniform_v3("u_light.diffuse", sun.diffuse);
            terrain_shader->set_uniform_v3("u_light.specular", sun.specular);
            terrain_shader->set_uniform_v3("u_light.direction", sun.direction);
            terrain_shader->set_uniform_m4("u_M", glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, 1.0f, 1.0f)));
            terrain_shader->set_uniform_m4("u_V", camera->get_view_matrix());
            terrain_shader->set_uniform_m4("u_P", camera->get_proj_matrix());
            terrain_shader->set_uniform_v3("u_viewPos", camera->position);
            terrain.draw(terrain_shader.get());


            shader->use();


            shader->set_uniform_v3("u_light_sun.ambient", sun.ambient);
            shader->set_uniform_v3("u_light_sun.diffuse", sun.diffuse);
            shader->set_uniform_v3("u_light_sun.specular", sun.specular);

            shader->set_uniform_v3("u_sunDir", sun.direction);


            shader->set_uniform_v3("u_light_point[0].ambient", point_light.ambient);
            shader->set_uniform_v3("u_light_point[0]diffuse", point_light.diffuse);
            shader->set_uniform_v3("u_light_point[0].specular", point_light.specular);

            shader->set_uniform_v3("u_lightPositions[0]", point_light.position);

            shader->set_uniform_f("u_light_point[0].K_c", point_light.K_c);
            shader->set_uniform_f("u_light_point[0].K_l", point_light.K_l);
            shader->set_uniform_f("u_light_point[0].K_q", point_light.K_q);


            shader->set_uniform_v3("u_cameraPos", camera->position);
            shader->set_uniform_m4("u_V", camera->get_view_matrix());
            if(update_projection){
                shader->set_uniform_m4("u_P", camera->get_proj_matrix());
                update_projection = false;
            }

            player.draw(0, *shader, current_time);
            //model->draw(*shader);


            particle_system.draw(smoke, width, height, camera);


            /*
            glDepthFunc(GL_LEQUAL);  // change depth function so depth test passes when values are equal to depth buffer's content
            skybox_shader->use();
            skybox_shader->set_uniform_i("u_skybox", 0);
            skybox_shader->set_uniform_m4("u_V", camera->get_view_matrix());
            skybox_shader->set_uniform_m4("u_P", camera->get_proj_matrix());
            skybox_shader->set_uniform_m4("u_M", glm::scale(glm::mat4(1.0f), glm::vec3(10000.0f)));
            // skybox cube
            glBindVertexArray(sky.VAO);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_CUBE_MAP, sky.texture);
            glDrawArrays(GL_TRIANGLES, 0, 36);
            glBindVertexArray(0);
            glDepthFunc(GL_LESS); // set depth function back to default
             */

        }
        else{
            deferred_shader->use();
            deferred_shader->set_uniform_i("u_gPos", 0);
            deferred_shader->set_uniform_i("u_gNormal", 1);
            deferred_shader->set_uniform_i("u_gAlbedoSpec", 2);

            /**
             * Start with the Geometry pass
             * In this pass we write to the g-buffer
             * We write positions, normals, speculars and diffuse in this buffer
             * so that we can use it in the lighting pass later
             * That way each geometry render won't have to check each light source for each pixel
             */
            glDisable(GL_BLEND);

            glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glm::mat4 model = glm::mat4(1.0f);
            gbuffer_shader->use();
            gbuffer_shader->set_uniform_m4("u_P", camera->get_proj_matrix());
            gbuffer_shader->set_uniform_m4("u_V", camera->get_view_matrix());
            for(const auto& pos : objectPositions){
                model = glm::mat4(1.0f);
                model = glm::translate(model, pos);
                model = glm::scale(model, glm::vec3(0.05f));
                gbuffer_shader->set_uniform_m4("model", model);
                player.draw(0, *gbuffer_shader, 0);
            }
            glBindFramebuffer(GL_FRAMEBUFFER, 0);

            /**
             * Use the information stored in the G-buffer to render a screen filling quad with the scene
             * The lighting is calculated here, only once per pixel.
             */
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            deferred_shader->use();
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, gPosition);
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, gNormal);
            glActiveTexture(GL_TEXTURE2);
            glBindTexture(GL_TEXTURE_2D, gAlbedoSpec);
            for(auto i{0}; i < lightPositions.size(); i++){
                //std::cout << "lightPos: " << lightPositions[i].x << ", " << lightPositions[i].y << ", " << lightPositions[i].z << std::endl;
                deferred_shader->set_uniform_v3("u_lights[" + std::to_string(i) + "].Position", lightPositions[i]);
                deferred_shader->set_uniform_v3("u_lights[" + std::to_string(i) + "].Color", lightColors[i]);
                // update attenuation parameters and calculate radius
                const float linear = 0.07;
                const float quadratic = 0.017;
                deferred_shader->set_uniform_f("u_lights[" + std::to_string(i) + "].Linear", linear);
                deferred_shader->set_uniform_f("u_lights[" + std::to_string(i) + "].Quadratic", quadratic);
            }
            deferred_shader->set_uniform_v3("u_viewPos", camera->position);
            renderQuad();



            /**
             * Copy the depth buffer from the G-Buffer to the default depth buffer so that we can use it later
             * The width/height is *2 because otherwise it only fills 1/4 of my retina screen
             *
             * Render a colored box for the light sources to show their locations
             * Since the depth buffer is copied, they shouls not cover everything
             */
            glBindFramebuffer(GL_READ_FRAMEBUFFER, gBuffer);
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
            glBlitFramebuffer(0, 0, width*2, height*2, 0, 0, width*2, height*2, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);

            light_box->use();
            light_box->set_uniform_m4("u_P", camera->get_proj_matrix());
            light_box->set_uniform_m4("u_V", camera->get_view_matrix());
            for (unsigned int i = 0; i < lightPositions.size(); i++){
                model = glm::mat4(1.0f);
                model = glm::translate(model, lightPositions[i]);
                model = glm::scale(model, glm::vec3(1.125f));
                light_box->set_uniform_m4("model", model);
                light_box->set_uniform_v3("u_lightColor", lightColors[i]);
                renderCube();
            }


        }
        
        /**
         * IMGUI AND THEN FINISH RENDERING
         */
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());


        glfwSwapBuffers(window);


        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    return 0;
}



unsigned int quadVAO = 0;
unsigned int quadVBO;
void renderQuad()
{
    if (quadVAO == 0)
    {
        float quadVertices[] = {
                // positions        // texture Coords
                -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
                -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
                1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
                1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        };
        // setup plane VAO
        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);
        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    }
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}

unsigned int cubeVAO = 0;
unsigned int cubeVBO = 0;

void renderCube()
{
    // initialize (if necessary)
    if (cubeVAO == 0)
    {
        float vertices[] = {
                // back face
                -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
                1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
                1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f, // bottom-right
                1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
                -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
                -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f, // top-left
                // front face
                -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
                1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f, // bottom-right
                1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
                1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
                -1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f, // top-left
                -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
                // left face
                -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
                -1.0f,  1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-left
                -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
                -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
                -1.0f, -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-right
                -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
                // right face
                1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
                1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
                1.0f,  1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-right
                1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
                1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
                1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-left
                // bottom face
                -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
                1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f, // top-left
                1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
                1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
                -1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f, // bottom-right
                -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
                // top face
                -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
                1.0f,  1.0f , 1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
                1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f, // top-right
                1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
                -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
                -1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f  // bottom-left
        };
        glGenVertexArrays(1, &cubeVAO);
        glGenBuffers(1, &cubeVBO);
        // fill buffer
        glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        // link vertex attributes
        glBindVertexArray(cubeVAO);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }

    /*
    glActiveTexture(GL_TEXTURE0);
    shader.set_uniform_i("texture_diffuse1", 0);
    glBindTexture(GL_TEXTURE_2D, textures[0]);
    glActiveTexture(GL_TEXTURE1);
    shader.set_uniform_i("texture_normal1", 1);
    glBindTexture(GL_TEXTURE_2D, textures[1]);
    glActiveTexture(GL_TEXTURE2);
    shader.set_uniform_i("texture_specular1", 2);
    glBindTexture(GL_TEXTURE_2D, textures[2]);*/

    // render Cube
    glBindVertexArray(cubeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
}
