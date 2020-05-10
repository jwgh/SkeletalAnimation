#ifndef SKELETAL_MESH_H
#define SKELETAL_MESH_H

#include <vector>
#include <memory>
#include <map>

#include <filesystem>

#include <assimp/scene.h>

#include "Shader.h"

constexpr unsigned int MAX_BONES{ 8 }; // 8 bones, because 4 is not enough and I'm sending it to the shader in vec4s
                                       // with only 4 bones influencing a vertex, the hands get all messed up because
                                       // of all the f-ing fingers.

// TODO: store texture and material info
struct Vertex {
    glm::vec3 position;
    glm::vec2 tex_coord;
    glm::vec3 normal;
    glm::vec3 tangent;
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
    GLuint normal0_ID;
    GLuint specular0_ID;
    GLuint white_ID;
    GLuint num_indices;
    GLuint num_vertices;

    std::string name;
    glm::mat4 default_transform;

    std::vector<std::shared_ptr<Mesh>> children;

    void setup_VAO(const std::vector<Vertex>& vertices, const std::vector<GLuint>& indices);
};

#endif //SKELETAL_MESH_H