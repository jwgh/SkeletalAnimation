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

/**
 * Model class holds the Nodes, Meshes, Materials and Animations in a scene imported from assimp
 *
 * initNode() recursively parses the info from an assimpScene and creates the tree of nodes
 * two draw functions are provided, one that just draws the default pos
 * and one that draws an animation, repeated
 *
 * update_bone_matrices recursively goes through the tree from root node and gets the TRS matrix for the skeletal animation
 *
 * Channel class holds the keyframe information, all the scaling and translation vectors and rotation quaternions for each keyframe
 *
 * Animation holds all the information for a single animation, like "Run"
 *
 * Node and KeyFrame classes are self explanatory
 *
 * interpolate T/R/S functions finds the two keyframes depending on time, and interpolates between them
 */

class Node{
public:
    std::string name;
    glm::mat4 transform;
    std::vector<std::shared_ptr<Node>> children;
};

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
    std::shared_ptr<Node> initNode(aiNode* ai_node, std::shared_ptr<Node> new_node);
    void update_bone_matrices(int animation_ID, std::shared_ptr<Node> node, const glm::mat4& transform, double ticks);

    static glm::mat4 interpolate_translation (const std::vector<KeyFramePos>& keys, double ticks);
    static glm::mat4 interpolate_rotation    (const std::vector<KeyFrameRot>& keys, double ticks);
    static glm::mat4 interpolate_scaling     (const std::vector<KeyFrameScale>& keys, double ticks);


    std::map<std::string, GLuint> bone_map;
    std::map<std::tuple<GLuint, std::string>, GLuint> anim_channels;

    std::map<GLuint, Animation> animations;

    GLuint total_bones{ 0 };
    std::vector<glm::mat4> bone_matrices;
    std::vector<glm::mat4> bone_offsets;

    std::filesystem::path dir_path;

    std::vector<std::shared_ptr<Mesh>> meshes;
    const aiScene* scene;

    std::shared_ptr<Node> root;
};

#endif //SKELETAL_MODEL_H