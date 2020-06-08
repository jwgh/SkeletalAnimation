//
// Created by Joakim Wingård on 2020-05-09.
//

#include "Player.h"
#include "glm/gtc/matrix_transform.hpp"

void Player::processMovement(Player::Movement m, double dt){
    float velocity = 50.0f * dt;

    // quat mult rotation q*P*q^(-1)
    // get front by rotating original orientation with current orientation
    glm::quat qF = orientation * glm::quat(0, 0, 0, -1) * glm::conjugate(orientation);
    glm::vec3 front{ qF.x, qF.y, qF.z };
    glm::vec3 right{glm::normalize(glm::cross(front, glm::vec3(0, 1, 0)))};
    
    if(m == Movement::FORWARD){
        pos -= front * velocity;
    }

    if(m == Movement::BACKWARD){
        pos += front * velocity;
    }

    if(m == Movement::STRAFT_LEFT){
        pos -= right * velocity;
    }

    if(m == Movement::STRAFE_RIGHT){
        pos += right * velocity;
    }

    if(m == Movement::TURN_LEFT){
        yaw += velocity*-4;
    }

    if(m == Movement::TURN_RIGHT){
        yaw -= velocity*-4;
    }
}

void Player::update(double dt){
    static glm::mat4 T{ 1.0f };
    T[3] = glm::vec4{pos, 1.0f };

    glm::quat qYaw = glm::angleAxis(glm::radians(-yaw), glm::vec3(0, 1, 0));
    orientation = qYaw;

    glm::mat4 S = glm::scale(glm::mat4(1.0f), glm::vec3(0.1f, 0.1f, 0.1f));
    M = T * glm::mat4(orientation) * S; // * R * S;
}

void Player::draw(GLuint animation_id, const Shader& shader, double time){
    shader.use();
    shader.set_uniform_m4("u_M", M);
    current_animation->draw(animation_id, shader, time);
}
