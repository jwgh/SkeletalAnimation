//
// Created by Joakim Wingård on 2020-05-10.
//

#ifndef SKELETAL_LIGHT_H
#define SKELETAL_LIGHT_H

#include <glm/glm.hpp>

class Light{
public:
    glm::vec3 ambient{ 0.3f };
    glm::vec3 diffuse{ 0.6f };
    glm::vec3 specular{ 1.0f };
};

class LightDirectional : public Light{
public:
    glm::vec3 direction{ -0.3f, -1.0f, -0.3f };
};

class LightPoint : public Light{
public:
    glm::vec3 position{ 0.0f };
    float K_c{ 1.0f };
    float K_l{ K_ls[9] };
    float K_q{ K_qs[9] };
    // values for attenuation for dist: 7      13      20     32     50      65    100       160      200       325     600      3250
    constexpr static float K_ls[12] = { 0.7f, 0.35f, 0.22f, 0.14f, 0.09f,  0.07f,  0.045f,  0.027f,  0.022f,  0.014f,  0.007f,  0.0014f };
    constexpr static float K_qs[12] = { 1.8f, 0.44f, 0.2f,  0.07f, 0.032f, 0.017f, 0.0075f, 0.0028f, 0.0019f, 0.0007f, 0.0002f, 0.000007f };

};


#endif //SKELETAL_LIGHT_H
