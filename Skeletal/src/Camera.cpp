//
// Created by Joakim Wingård on 2020-04-20.
//

#include "Camera.h"


Camera::Camera(glm::vec3 pos, uint32_t w, uint32_t h) :
        speed(DEFAULT_SPEED),
        sensitivity(DEFAULT_SENSITIVITY),
        zoom(DEFAULT_ZOOM),
        position(pos),
        orientation(glm::quat(0, 0, 0, -1)),
        yaw(0.0f),
        pitch(0.0f),
        wh_ratio{(float)w / (float) h}{
    updateCameraVectors();
}

const glm::mat4 Camera::get_view_matrix() const{
    glm::quat reverseOrient = glm::conjugate(orientation); // reverse of eye direction
    glm::mat4 r = glm::mat4_cast(reverseOrient);           // as a rotation matrix
    // translated to the camera position
    glm::mat4 t = glm::translate(glm::mat4(1.0), -position);
    return r * t;                                          // lookAt = orientation(right, up, down) * translation
}

void Camera::processKeyboard(const Movement direction, const float deltaTime){
    float velocity{speed * deltaTime};

    // quat mult rotation q*P*q^(-1)
    // get front by rotating original orientation with current orientation
    glm::quat qF = orientation * glm::quat(0, 0, 0, -1) * glm::conjugate(orientation);
    glm::vec3 front{ qF.x, qF.y, qF.z };
    glm::vec3 right{glm::normalize(glm::cross(front, glm::vec3(0, 1, 0)))};

    if(direction == Movement::FORWARD){
        position += front * velocity;
    }

    if(direction == Movement::BACKWARD){
        position -= front * velocity;
    }

    if(direction == Movement::LEFT){
        position -= right * velocity;
    }

    if(direction == Movement::RIGHT){
        position += right * velocity;
    }
}

void Camera::processMouseMovement(float xoffset, float yoffset, const bool constrainPitch){
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch += yoffset;

    if(constrainPitch){
        if(pitch > 89.0f){
            pitch = 89.0f;
        }
        if(pitch < -89.0f){
            pitch = -89.0f;
        }
    }

    updateCameraVectors();
}

void Camera::processMouseScroll(const float yoffset){
    if(zoom >= 1.0f && zoom <= 45.0f){
        zoom -= yoffset;
    }
    if(zoom <= 1.0f){
        zoom = 1.0f;
    }
    if(zoom >= 45.0f){
        zoom = 45.0f;
    }
}

void Camera::updateCameraVectors()
{
    // Yaw (rotation around y axis)
    glm::quat qYaw = glm::angleAxis(glm::radians(-yaw), glm::vec3(0, 1, 0));

    // Pitch (rotation around x axis)
    glm::quat qPitch = glm::angleAxis(glm::radians(pitch), glm::vec3(1, 0, 0));

    // Roll (rotation around z axis), an be omitted for an FPS style camera

    // Quat mult is NOT commutative, however several different orders would behave well here.
    // (yaw*pitch*roll) or (roll*yaw*pitch) or (yaw*roll*pitch)
    orientation = qYaw * qPitch; // * qRoll
}

const glm::mat4 Camera::get_proj_matrix() const{
    return glm::perspective(glm::radians(45.0f), 1.0f * (float) wh_ratio, 0.1f, 1000.0f);
}

void Camera::update_ratio(uint32_t w, uint32_t h){
    wh_ratio = (float) w / (float) h;
}
