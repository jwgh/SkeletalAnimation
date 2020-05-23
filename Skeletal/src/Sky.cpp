//
// Created by Joakim Wingård on 2020-05-23.
//
#include <glad/glad.h>
#include <vector>

#include "Sky.h"
#include "TextureManager.h"

Sky::Sky(){
    //texture = TextureManager::load_single_color_texture(100, 127, 255);

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skybox_vertices), &skybox_vertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);


    std::vector<std::filesystem::path> faces
            {
                    "../Resources/textures/skybox/right.jpg",
                    "../Resources/textures/skybox/left.jpg",
                    "../Resources/textures/skybox/top.jpg",
                    "../Resources/textures/skybox/bottom.jpg",
                    "../Resources/textures/skybox/front.jpg",
                    "../Resources/textures/skybox/back.jpg"
            };
    texture = TextureManager::load_cubemap_from_files(faces);
}
