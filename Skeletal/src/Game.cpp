//
// Created by Joakim Wingård on 2020-05-30.
//

#include "Game.h"

Game::Game(){
    for(auto i{0}; i < TOTAL_BOXES; i++){

        std::random_device rd;
        std::mt19937 gen(rd());
        static std::uniform_real_distribution<> pos_dist(0.0f, 800.0f);
        static std::uniform_real_distribution<> col_dist(0.0f, 1.0f);
        glm::vec3 angle{col_dist(gen), col_dist(gen), col_dist(gen)};
        boxes.emplace_back(Box{
                glm::vec3(pos_dist(gen), 10.0f, pos_dist(gen)),
                glm::quat(-1.0f, glm::normalize(angle)),
                glm::vec3(col_dist(gen), col_dist(gen), col_dist(gen))
        });
    }
}
