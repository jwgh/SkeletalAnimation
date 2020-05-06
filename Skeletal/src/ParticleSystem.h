#ifndef SKELETAL_PARTICLESYSTEM_H
#define SKELETAL_PARTICLESYSTEM_H
#include <glad/glad.h>
#include <vector>
#include <glm/glm.hpp>

struct Particle {
	float lifetime    { 0.0f };
	float life_length { 10.0f };
	glm::vec3 velocity{ 0.0f };
	glm::vec3 pos     { 0.0f };
};

using ReducedParticle = glm::vec4;

class ParticleSystem {
 public:
	// Members
	std::vector<Particle> particles;
	std::vector<ReducedParticle> reduced_particles;
	int max_size;
	ParticleSystem() = delete;

	explicit ParticleSystem(int size) : max_size(size) {
        particles.resize(max_size);
        reduced_particles.resize(max_size);
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        //glGenBuffers(1, &EBO);
        glBindVertexArray(VAO);

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        // nullptr
        glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec4) * max_size, nullptr, GL_STATIC_DRAW);

        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        glBindBuffer(GL_ARRAY_BUFFER, 0);

        glBindVertexArray(0);
	}
	~ParticleSystem() {}
	// Methods
	void kill(int id);
	void spawn(Particle& particle);
	void process_particles(float dt);

	GLuint VAO, VBO, EBO;
};

#endif //SKELETAL_MODEL_H