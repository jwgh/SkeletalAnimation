#ifndef SKELETAL_MESH_H
#define SKELETAL_MESH_H

#include <vector>
#include <memory>
#include <map>

#include <filesystem>

#include <assimp/scene.h>

#include "Shader.h"

constexpr unsigned int MAX_BONES{ 8 };

// TODO: store texture and material info
// TODO: move bone stuff from aiscene
struct Vertex {
    glm::vec3 position;
    glm::vec2 tex_coord;
    glm::vec3 normal;
    int bone_ids[MAX_BONES] {0, 0, 0, 0, 0, 0, 0, 0};
    float bone_weights[MAX_BONES] {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
};

class Mesh {
public:
    Mesh() = default;
    ~Mesh();
    void draw(const Shader& shader_ptr) const;

    void add_bone(Vertex& vertex, int boneID, float weight);

    GLuint VAO;
    GLuint VBO;
    GLuint EBO;
    GLuint diffuse0_ID;
    GLuint num_indices;
    GLuint num_vertices;

    std::string name;
    glm::mat4 default_transform;

    std::vector<std::shared_ptr<Mesh>> children;

    void setup_VAO(const std::vector<Vertex>& vertices, const std::vector<GLuint>& indices);
};

#endif //SKELETAL_MESH_H