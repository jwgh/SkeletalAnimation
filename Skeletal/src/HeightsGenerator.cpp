//
// Created by Joakim Wingård on 2020-05-18.
//

#include <iostream>
#include "HeightsGenerator.h"

HeightsGenerator::HeightsGenerator() : seed{ 42 }{
}

float HeightsGenerator::generate_height(float x, float z){

    return (PerlinNoise::noise(x*0.00625, z*0.00625, 5.0*0.00625) * AMPLITUDE_MAJOR +
            PerlinNoise::noise(x*0.0125, z*0.0125, 5.0*0.0125) * AMPLITUDE_MIDDLE +
            PerlinNoise::noise(x*0.025, z*0.025, 5.0*0.025) * AMPLITUDE_MINOR) * 2.0f - 1.0f;
    return get_interpolated_noise(x * 0.125f, z * 0.125f) * AMPLITUDE_MAJOR +
            get_interpolated_noise(x * 0.25f, z * 0.25f) * AMPLITUDE_MINOR ;
}

float HeightsGenerator::get_noise(int x, int z){
    //static std::random_device rd;
    std::mt19937 gen(x*42 + z*66 + seed);
    static std::uniform_real_distribution<> dist(-1.0f, 1.0f);
    return dist(gen);
}

float HeightsGenerator::cos_interpolate(float a, float b, float blend){
    float theta{ blend * 3.14159f };
    float f = (1.0f - std::cos(theta)) * 0.5f;
    return a * (1.0f - f) + b * f;
}

float HeightsGenerator::get_smooth_noise(int x, int z){
    float corner_value{ (get_noise(x-1, z-1) + get_noise(x+1, z+1) + get_noise(x-1, z+1) + get_noise(x+1, z-1)) / 16.0f };
    float side_value{ (get_noise(x, z-1) + get_noise(x, z+1) + get_noise(x-1, z) + get_noise(x+1, z)) / 8.0f };
    float center_value{ get_noise(x, z) / 4.0f };
    return (corner_value + side_value + center_value) * AMPLITUDE_MAJOR;
}

float HeightsGenerator::get_interpolated_noise(float x, float z){
    int x_i{ static_cast<int>(x) };
    int z_i{ static_cast<int>(z) };
    float x_frac{ x - x_i };
    float z_frac{ z - z_i };

    float v1{ get_smooth_noise(x_i, z_i) };
    float v2{ get_smooth_noise(x_i+1, z_i) };
    float v3{ get_smooth_noise(x_i, z_i+1) };
    float v4{ get_smooth_noise(x_i+1, z_i+1) };

    float i1 = cos_interpolate(v1, v2, x_frac);
    float i2 = cos_interpolate(v3, v4, x_frac);

    return cos_interpolate(i1, i2, z_frac);
}
