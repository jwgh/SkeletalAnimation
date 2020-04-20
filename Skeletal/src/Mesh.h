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
    Mesh(const std::filesystem::path& directory_path, aiMesh* mesh, const aiScene* scene,
         std::map<std::string, GLuint>& bone_map, GLuint& total_bones, std::vector<glm::mat4>& bone_offsets);
    ~Mesh();
    void draw(const Shader& shader_ptr) const;

private:
    void add_bone(Vertex& vertex, int boneID, float weight);

    GLuint VAO;
    GLuint VBO;
    GLuint EBO;
    GLuint texID;
    GLuint num_indices;
    GLuint num_vertices;
};

#endif //SKELETAL_MESH_H