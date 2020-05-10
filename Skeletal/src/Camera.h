//
// Created by Joakim Wingård on 2020-04-20.
//

#ifndef SKELETAL_CAMERA_H
#define SKELETAL_CAMERA_H
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


constexpr float DEFAULT_SPEED{5.0f};
constexpr float DEFAULT_SENSITIVITY{0.5f};
constexpr float DEFAULT_ZOOM{45.0f};

class Camera{
public:
    enum class Movement{
        FORWARD,
        BACKWARD,
        LEFT,
        RIGHT
    };

    glm::vec3 position;

    glm::quat orientation;

    float yaw;
    float pitch;

    float speed;
    float sensitivity;
    float zoom;
    float wh_ratio;

    explicit Camera(glm::vec3 pos = glm::vec3(0.f, 0.f, 0.f), uint32_t w = 800, uint32_t h = 600);

    const glm::mat4 get_view_matrix() const;
    const glm::mat4 get_proj_matrix() const;
    void processKeyboard(const Movement direction, const float deltaTime);
    void processMouseMovement(float xoffset, float yoffset, const bool constrainPitch = true);
    void processMouseScroll(const float yoffset);
    void update_ratio(uint32_t w, uint32_t h);

private:
    void updateCameraVectors();
};

#endif //SKELETAL_CAMERA_H
