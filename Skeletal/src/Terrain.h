//
// Created by Joakim Wingård on 2020-05-18.
//

#ifndef SKELETAL_TERRAIN_H
#define SKELETAL_TERRAIN_H

struct TerrainVertex{
    float pos[3];
    float normal[3];
    float UV[2];
};

struct RawModel{
    unsigned int VAO;
    unsigned int vertex_count;
    unsigned int texture;
};

class Terrain{
public:
    static constexpr double SIZE { 800.0 };
    static constexpr unsigned int VERTEX_COUNT { 128 };

    Terrain() = delete;
    Terrain(unsigned int grid_x, unsigned int grid_z);

private:
    double x;
    double z;

    RawModel model;

    RawModel generateTerrain();
    //Texture texture;


};


#endif //SKELETAL_TERRAIN_H
