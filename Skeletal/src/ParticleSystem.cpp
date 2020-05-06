#include "ParticleSystem.h"
#include <random>

ParticleSystem::ParticleSystem(std::shared_ptr<Shader> shader, int size) : shader{ shader }, max_size(size) {
    particles.resize(max_size);
    reduced_particles.resize(max_size);
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec4) * max_size, nullptr, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void ParticleSystem::kill(int id){
    particles[id] = particles.back();
    particles.pop_back();
}

void ParticleSystem::spawn(Particle& particle){
    if(particles.size() >= max_size){
        return;
    }
    particles.push_back(particle);
}

void ParticleSystem::process_particles(float dt){
    for(auto i{0}; i < particles.size(); i++){
        auto& p = particles[i];
        if(p.lifetime / p.life_length >= 1.0f){
            kill(i);
        }
    }
    for(auto& p : particles){
        p.lifetime += dt;
        p.velocity.y -= 0.0987f;
        p.pos += dt * p.velocity;
    }
}

void ParticleSystem::update(double dt){
    static auto unit_sphere_rand_point= []() -> glm::vec3{
        glm::vec3 result{0.0f};
        static std::random_device rd;
        static std::mt19937 gen(rd());
        static std::uniform_real_distribution<> dist(-1.0f, 1.0f);
        result.x = dist(gen);
        result.y = dist(gen);
        result.z = dist(gen);
        return glm::normalize(result);
    };

    for(auto i{0}; i < 64; i++){
        static auto p = Particle();
        p.pos = unit_sphere_rand_point() * 10.0f;
        p.velocity = unit_sphere_rand_point() * 25.0f;
        p.velocity += glm::vec3(0.0f, 75.0f, 0.0f);
        spawn(p);
    }
    process_particles(dt);
    unsigned int active_particles = particles.size();
    std::vector<glm::vec4> data;
    data.resize(active_particles);
    for(auto i{0}; i < active_particles; i++){
        data[i].x = particles[i].pos.x;
        data[i].y = particles[i].pos.y;
        data[i].z = particles[i].pos.z;
        data[i].w = particles[i].lifetime;
    }

    std::sort(data.begin(), std::next(data.begin(), active_particles),
              [](const glm::vec4 &lhs, const glm::vec4 &rhs){ return lhs.z < rhs.z; });

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(glm::vec4)*active_particles, data.data());
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void ParticleSystem::draw(GLuint texture, GLuint w, GLuint h, const std::shared_ptr<Camera>& camera){
    glBindVertexArray(VAO);
    shader->use();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    shader->set_uniform_i("colortexture", 0);
    shader->set_uniform_f("screen_x", static_cast<float>(w));
    shader->set_uniform_f("screen_y", static_cast<float>(h));
    shader->set_uniform_m4("M", glm::mat4{1.0f});
    shader->set_uniform_m4("V", camera->get_view_matrix());
    shader->set_uniform_m4("P", camera->get_proj_matrix());
    glDrawArrays(GL_POINTS, 0, reduced_particles.size());
    glBindVertexArray(0);
}


