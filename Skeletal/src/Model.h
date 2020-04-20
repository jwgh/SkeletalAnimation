#ifndef SKELETAL_MODEL_H
#define SKELETAL_MODEL_H

#include <map>
#include <memory>
#include <vector>
#include <string>
#include <filesystem>

#include <assimp/scene.h>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include "Mesh.h"
#include "Shader.h"


struct KeyFramePos{
    double time;
    glm::vec3 vec;
}; using KeyFrameScale = KeyFramePos;

struct KeyFrameRot{
    double time;
    glm::quat quat;
};

class Channel{
public:
    GLuint ID;
    std::string node_name;
    std::vector<KeyFramePos> keyframes_position;
    std::vector<KeyFrameRot> keyframes_rotation;
    std::vector<KeyFrameScale> keyframes_scaling;
    GLuint num_keyframes_position;
    GLuint num_keyframes_rotation;
    GLuint num_keyframes_scaling;
};

class Animation{
public:
    std::string name;
    GLuint ID;
    double duration;
    double ticks_per_second;
    GLuint num_channels;
    std::vector<Channel> channels;
};

class Model {
public:
    Model() = delete;
    Model(const std::filesystem::path& path);
    ~Model();
    void draw(const Shader& shader);
    void draw(GLuint animation_id, const Shader& shader, double time);
    
private:
    void initNode(aiNode* node);
    void update_bone_matrices(int animation_id, aiNode* node, const glm::mat4& transform, double ticks);
    
    static glm::mat4 interpolate_translation(aiVectorKey* keys, GLuint n, double ticks);
    static glm::mat4 interpolate_rotation   (aiQuatKey*   keys, GLuint n, double ticks);
    static glm::mat4 interpolate_scaling    (aiVectorKey* keys, GLuint n, double ticks);


    std::map<std::string, GLuint> bone_map;
    std::map<std::tuple<GLuint, std::string>, GLuint> anim_channels;

    std::map<GLuint, Animation> animations;

    GLuint total_bones{ 0 };
    std::vector<glm::mat4> bone_matrices;
    std::vector<glm::mat4> bone_offsets;

    std::filesystem::path dir_path;

    std::vector<std::shared_ptr<Mesh>> meshes;
    const aiScene* scene;
};

#endif //SKELETAL_MODEL_H