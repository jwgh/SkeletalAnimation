#include "ParticleSystem.h"

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
    }
}
