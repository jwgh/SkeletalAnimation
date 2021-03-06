//
// Created by Joakim Wingård on 2020-05-18.
//

#ifndef SKELETAL_HEIGHTSGENERATOR_H
#define SKELETAL_HEIGHTSGENERATOR_H

#include <random>
#include "PerlinNoise.h"
class HeightsGenerator{
public:
    static constexpr float AMPLITUDE_MAJOR{ 24.0F };
    static constexpr float AMPLITUDE_MIDDLE{ 12.0F };
    static constexpr float AMPLITUDE_MINOR{ 6.0F };
    int seed;

    HeightsGenerator();

    float generate_height(float x, float z);
    float get_noise(int x, int z);
    float get_smooth_noise(int x, int z);
    float get_interpolated_noise(float x, float z);


    float cos_interpolate(float a, float b, float blend);
};


#endif //SKELETAL_HEIGHTSGENERATOR_H
