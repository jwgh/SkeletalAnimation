#ifndef SKELETAL_PARTICLESYSTEM_H
#define SKELETAL_PARTICLESYSTEM_H
#include <glad/glad.h>
#include <vector>
#include <glm/glm.hpp>
#include "Shader.h"
#include "Camera.h"

struct Particle {
	float lifetime    { 0.0f };
	float life_length { 2.0f };
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

	ParticleSystem(std::shared_ptr<Shader> shader, int size);
	~ParticleSystem() {}
	// Methods
	void kill(int id);
	void spawn(Particle& particle);
	void process_particles(float dt);
	void update(double dt, bool key);
	void draw(GLuint texture, GLuint w, GLuint h, const std::shared_ptr<Camera>& camera);

	GLuint VAO, VBO, EBO;
	std::shared_ptr<Shader> shader;
};

#endif //SKELETAL_MODEL_H