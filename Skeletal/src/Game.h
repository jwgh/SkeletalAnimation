//
// Created by Joakim Wingård on 2020-05-30.
//

#ifndef SKELETAL_GAME_H
#define SKELETAL_GAME_H
#include <vector>
#include <random>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

/**
 *
 */

struct Box{
    glm::vec3 pos{0.0f};
    glm::quat orientation{0.0f ,0.0f ,0.0f ,-1.0f};
    glm::vec3 color{1.0f, 0.0f, 0.0f};
    bool collected{false};
};
class Game{
public:
    Game();
    static constexpr float TOTAL_TIME{60.0f};
    static constexpr unsigned int TOTAL_BOXES{10};
    float time{0.0f};
    float time_between_boxes{0.0f};
    unsigned int collected{0};
    std::vector<Box> boxes;
    double score{0.0};
};


#endif //SKELETAL_GAME_H
