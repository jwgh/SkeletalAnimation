//
// Created by Joakim Wingård on 2020-05-09.
//

#ifndef SKELETAL_PLAYER_H
#define SKELETAL_PLAYER_H
#include <glm/glm.hpp>
#include <glad/glad.h>
#include "Model.h"

class Player{
public:
    enum class Movement{
        FORWARD,
        BACKWARD,
        TURN_LEFT,
        STRAFT_LEFT,
        TURN_RIGHT,
        STRAFE_RIGHT
    };

    void processMovement(Movement m, double dt);

    void update(double dt);

    void draw(GLuint animation_id, const Shader& shader, double time);

    glm::mat4 M{ 1.0f };

    glm::vec3 pos{ 0.0f };
    float yaw{ 0.0f };
    glm::quat orientation { 0.0f ,0.0f, 0.0f, -1.0f };

    Model* idle{ nullptr };
    Model* run{ nullptr };
    Model* current_animation{ nullptr };
};


#endif //SKELETAL_PLAYER_H
