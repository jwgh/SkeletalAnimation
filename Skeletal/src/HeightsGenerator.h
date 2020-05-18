//
// Created by Joakim Wingård on 2020-05-18.
//

#ifndef SKELETAL_HEIGHTSGENERATOR_H
#define SKELETAL_HEIGHTSGENERATOR_H

#include <random>
class HeightsGenerator{
public:
    static constexpr float AMPLITUDE{ 70.0F };
    /*static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_real_distribution<> dist(-1.0f, 1.0f);*/
    int seed;

    HeightsGenerator();

    float generate_height(int x, int y);
};


#endif //SKELETAL_HEIGHTSGENERATOR_H
