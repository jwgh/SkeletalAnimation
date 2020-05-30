//
// Created by Joakim Wingård on 2020-05-18.
//

#ifndef SKELETAL_TERRAIN_H
#define SKELETAL_TERRAIN_H

#include "Shader.h"
#include "HeightsGenerator.h"

struct TerrainVertex{
    glm::vec3 pos;
    glm::vec3 normal;
    glm::vec2 UV;
};

struct RawModel{
    unsigned int VAO;
    unsigned int vertex_count;
    unsigned int num_indices;
    unsigned int texture;
};

class Terrain{
public:
    static constexpr double SIZE { 800.0 };
    static constexpr unsigned int VERTEX_COUNT { 128 };

    Terrain() = delete;
    Terrain(unsigned int grid_x, unsigned int grid_z);

    void draw(Shader* shader);

    float get_height(int x, int z);

private:
    double x;
    double z;

    RawModel model;

    RawModel generateTerrain();

    HeightsGenerator heights_generator;


    glm::vec3 calc_normal(int x, int z);
    //Texture texture;


};


#endif //SKELETAL_TERRAIN_H
