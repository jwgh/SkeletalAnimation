//
// Created by Joakim Wingård on 2020-05-18.
//
#include <glad/glad.h>
#include <iostream>
#include "Terrain.h"
#include "TextureManager.h"

Terrain::Terrain(unsigned int grid_x, unsigned int grid_z): x{ grid_x * SIZE }, z{ grid_z * SIZE }{
    model = generateTerrain();
}

RawModel Terrain::generateTerrain(){
    static const unsigned int count{ VERTEX_COUNT * VERTEX_COUNT };
    static const unsigned int indices_size{ 6 * (VERTEX_COUNT-1) * (VERTEX_COUNT-1) };
    TerrainVertex vertices[count];
    unsigned int indices[indices_size];
    unsigned int vert_ptr{ 0 };

    for(auto i{0}; i < VERTEX_COUNT; i++){
        for(auto j{0}; j < VERTEX_COUNT; j++){
            vertices[vert_ptr].pos[0] = static_cast<float>(j) / (static_cast<float>(VERTEX_COUNT - 1)) * SIZE;
            vertices[vert_ptr].pos[1] = 0;
            vertices[vert_ptr].pos[2] = static_cast<float>(i) / (static_cast<float>(VERTEX_COUNT - 1)) * SIZE;
            vertices[vert_ptr].normal[0] = 0;
            vertices[vert_ptr].normal[1] = 1;
            vertices[vert_ptr].normal[2] = 0;
            vertices[vert_ptr].UV[0] = static_cast<float>(j) / (static_cast<float>(VERTEX_COUNT - 1));
            vertices[vert_ptr].UV[1] = static_cast<float>(i) / (static_cast<float>(VERTEX_COUNT - 1));
            vert_ptr++;
        }
    }

    unsigned int ptr{ 0 };
    for(auto gz{0}; gz < VERTEX_COUNT-1; gz++){
        for(auto gx{0}; gx < VERTEX_COUNT-1; gx++){
            unsigned int top_left = (gz*VERTEX_COUNT)+gx;
            unsigned int top_right = top_left + 1;
            unsigned int btm_left = ((gz+1)*VERTEX_COUNT)+gx;
            unsigned int btm_right = btm_left + 1;
            indices[ptr++] = top_left;
            indices[ptr++] = btm_left;
            indices[ptr++] = top_right;
            indices[ptr++] = top_right;
            indices[ptr++] = btm_left;
            indices[ptr++] = btm_right;
        }
    }

    unsigned int VAO;
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    unsigned int VBO;
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(TerrainVertex) * count, vertices, GL_STATIC_DRAW);

    unsigned int EBO;
    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * indices_size, &indices[0], GL_STATIC_DRAW);


    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, false, sizeof(TerrainVertex), (void *) offsetof(TerrainVertex, pos));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, false, sizeof(TerrainVertex), (void *) offsetof(TerrainVertex, UV));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, false, sizeof(TerrainVertex), (void *) offsetof(TerrainVertex, normal));

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    GLuint texture_color = TextureManager::load_single_color_texture(42, 120, 42);

    std::cout << "texture:" << texture_color << std::endl;
    return RawModel{VAO, count, indices_size, texture_color};
}

void Terrain::draw(Shader* shader){
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, model.texture);

    shader->set_uniform_i("u_Diffuse0", 0);

    glBindVertexArray(model.VAO);
    glDrawElements(GL_TRIANGLES, model.num_indices, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
    glActiveTexture(GL_TEXTURE0);

}


