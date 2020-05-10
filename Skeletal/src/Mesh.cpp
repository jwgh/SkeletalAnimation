#include <algorithm>

#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>

#include "Mesh.h"
#include "TextureManager.h"
#include "Util.h"

Mesh::~Mesh() {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteTextures(1, &diffuse0_ID);
}

void Mesh::draw(const Shader& shader) const {
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, diffuse0_ID);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, normal0_ID);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, specular0_ID);


    //shader.set_uniform_v3("u_light_color", glm::vec3{1.0f, 1.0f, 0.0f});
    //shader.set_uniform_v3("u_material.ambient", glm::vec3{1.0f, 1.0f, 1.0f});
    //shader.set_uniform_v3("u_material.diffuse", glm::vec3{1.0f, 1.0f, 1.0f});
    shader.set_uniform_i("u_material.diffuse", 0);
    shader.set_uniform_i("u_material.normal", 1);
    shader.set_uniform_i("u_material.specular", 2);
    shader.set_uniform_v3("u_material.specular", glm::vec3{1.0f, 1.0f, 1.0f});
    shader.set_uniform_f("u_material.shininess", 32.0f);
    
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, num_indices, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
    glActiveTexture(GL_TEXTURE0);
}

void Mesh::add_bone(Vertex& vertex, int boneID, float weight){
    int i{ 0 };
    for(; i < MAX_BONES; i++){
        if(vertex.bone_weights[i] == 0){
            break;
        }
    }
    vertex.bone_weights[i] = weight;
    vertex.bone_ids[i] = boneID;
}

void Mesh::setup_VAO(const std::vector<Vertex>& vertices, const std::vector<GLuint>& indices){
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * vertices.size(), vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * indices.size(), indices.data(), GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, false, sizeof(Vertex), (void *) offsetof(Vertex, position));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, false, sizeof(Vertex), (void *) offsetof(Vertex, tex_coord));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, false, sizeof(Vertex), (void *) offsetof(Vertex, normal));

    glEnableVertexAttribArray(3);
    glVertexAttribIPointer(3, 4, GL_INT, sizeof(Vertex), (void *) (offsetof(Vertex, bone_ids) + 0 * sizeof(int)));
    glEnableVertexAttribArray(4);
    glVertexAttribIPointer(4, 4, GL_INT, sizeof(Vertex), (void *) (offsetof(Vertex, bone_ids) + 4 * sizeof(int)));

    glEnableVertexAttribArray(5);
    glVertexAttribPointer(5, 4, GL_FLOAT, false, sizeof(Vertex), (void *) (offsetof(Vertex, bone_weights) + 0 * sizeof(float)));
    glEnableVertexAttribArray(6);
    glVertexAttribPointer(6, 4, GL_FLOAT, false, sizeof(Vertex), (void *) (offsetof(Vertex, bone_weights) + 4 * sizeof(float)));

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);


    white_ID = TextureManager::load_single_color_texture(255, 255, 255);
}
