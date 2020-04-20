#include <iostream>

#include <glad/glad.h>
#include <assimp/cimport.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <glm/gtc/matrix_transform.hpp>

#include "Model.h"
#include "Util.h"

Model::Model(const std::filesystem::path& path): dir_path(path.parent_path()) {
    scene = aiImportFile(path.c_str(),
            aiProcess_Triangulate |
            aiProcess_JoinIdenticalVertices |
            aiProcess_GenUVCoords |
            aiProcess_SortByPType |
            aiProcess_RemoveRedundantMaterials | // is it this one??
            aiProcess_FindInvalidData |
            aiProcess_FlipUVs |
            aiProcess_CalcTangentSpace |
            aiProcess_GenSmoothNormals |
            aiProcess_ImproveCacheLocality |
            aiProcess_OptimizeMeshes |
            aiProcess_SplitLargeMeshes
    );

    anim_channels.clear();
    for (auto i{ 0 }; i < scene->mNumAnimations; i++) {
        auto animation = scene->mAnimations[i];
        for (auto j{ 0 }; j < animation->mNumChannels; j++) {
            auto channel = animation->mChannels[j];
            anim_channels[std::tuple<GLuint, std::string>(i, channel->mNodeName.C_Str())] = j;
        }
    }

    initNode(scene->mRootNode);
    bone_matrices.resize(total_bones);
}

Model::~Model() {
    aiReleaseImport(scene);
}

void Model::initNode(aiNode* node) {
    for (auto i{ 0 }; i < node->mNumMeshes; i++) {
        auto mesh = scene->mMeshes[node->mMeshes[i]];
        meshes.emplace_back(std::make_shared<Mesh>(dir_path, mesh, scene, bone_map, total_bones, bone_offsets));
    }
    for (auto i{ 0 }; i < node->mNumChildren; i++) {
        initNode(node->mChildren[i]);
    }
}

glm::mat4 Model::interpolate_translation(aiVectorKey* keys, GLuint n, double ticks) {
    if (n == 0){
        return glm::mat4{ 1.0f };
    }
    if (n == 1){
        return glm::translate(glm::mat4{ 1.0f }, glm::vec3(keys->mValue.x, keys->mValue.y, keys->mValue.z));
    }
    if (ticks <= keys[0].mTime){
        return glm::translate(glm::mat4{ 1.0f }, glm::vec3(keys[0].mValue.x, keys[0].mValue.y, keys[0].mValue.z));
    }
    if (keys[n - 1].mTime <= ticks){
        return glm::translate(glm::mat4{ 1.0f },
                              glm::vec3(keys[n - 1].mValue.x, keys[n - 1].mValue.y, keys[n - 1].mValue.z));
    }

    aiVectorKey anchor;
    anchor.mTime = ticks;
    auto right_ptr = std::upper_bound(keys, keys + n, anchor, [] (const aiVectorKey &a, const aiVectorKey &b) {
        return a.mTime < b.mTime;
    });
    auto left_ptr = right_ptr - 1;
    
    float factor = (ticks - left_ptr->mTime) / (right_ptr->mTime - left_ptr->mTime);

    static aiVector3D interpolated = left_ptr->mValue * (1.0f - factor) + right_ptr->mValue * factor;
    return glm::translate(glm::mat4{ 1.0f }, glm::vec3(interpolated.x, interpolated.y, interpolated.z));
}

glm::mat4 Model::interpolate_rotation(aiQuatKey* keys, GLuint n, double ticks) {
    if (n == 0) {
        return glm::mat4{ 1.0f };
    }
    if (n == 1) {
        return assimp_to_glm_mat4(aiMatrix4x4(keys->mValue.GetMatrix()));
    }
    if (ticks <= keys[0].mTime){
        return assimp_to_glm_mat4(aiMatrix4x4(keys[0].mValue.GetMatrix()));
    }
    if (keys[n - 1].mTime <= ticks) {
        return assimp_to_glm_mat4(aiMatrix4x4(keys[n - 1].mValue.GetMatrix()));
    }
    
    aiQuatKey anchor;
    anchor.mTime = ticks;
    auto right_ptr = std::upper_bound(keys, keys + n, anchor, [] (const aiQuatKey &a, const aiQuatKey &b) {
        return a.mTime < b.mTime;
    });
    auto left_ptr = right_ptr - 1;
    
    double factor = (ticks - left_ptr->mTime) / (right_ptr->mTime - left_ptr->mTime);
    aiQuaternion out;
    aiQuaternion::Interpolate(out, left_ptr->mValue, right_ptr->mValue, factor);

    return assimp_to_glm_mat4(aiMatrix4x4(out.GetMatrix()));
}

glm::mat4 Model::interpolate_scaling(aiVectorKey* keys, GLuint n, double ticks) {
    if (n == 0){
        return glm::mat4{ 1.0f }; }

    if (n == 1){
        return glm::scale(glm::mat4{ 1.0f }, glm::vec3{ keys->mValue.x, keys->mValue.y, keys->mValue.z });
    }
    if (ticks <= keys[0].mTime){
        return glm::scale(glm::mat4{ 1.0f }, glm::vec3{ keys[0].mValue.x, keys[0].mValue.y, keys[0].mValue.z });
    }
    if (keys[n - 1].mTime <= ticks){
        return glm::scale(glm::mat4{ 1.0f },
                          glm::vec3{ keys[n - 1].mValue.x, keys[n - 1].mValue.y, keys[n - 1].mValue.z });
    }
    
    aiVectorKey anchor;
    anchor.mTime = ticks;
    auto right_ptr = std::upper_bound(keys, keys + n, anchor, [] (const aiVectorKey &a, const aiVectorKey &b) {
        return a.mTime < b.mTime;
    });
    auto left_ptr = right_ptr - 1;
    
    float factor = (ticks - left_ptr->mTime) / (right_ptr->mTime - left_ptr->mTime);

    aiVector3D interpolated = left_ptr->mValue * (1.0f - factor) + right_ptr->mValue * factor;
    return glm::scale(glm::mat4{ 1.0f }, glm::vec3{ interpolated.x, interpolated.y, interpolated.z });
}

void Model::update_bone_matrices(int animation_id, aiNode* node, const glm::mat4& transform, double ticks) {
    std::string node_name = node->mName.C_Str();
    auto animation = scene->mAnimations[animation_id];
    glm::mat4 current_transform;
    if (anim_channels.count(std::tuple<GLuint, std::string>(animation_id, node_name))) {
        GLuint channel_id = anim_channels[std::tuple<GLuint, std::string>(animation_id, node_name)];
        auto channel = animation->mChannels[channel_id];
    
        // translation matrix
        glm::mat4 translation_matrix = interpolate_translation(channel->mPositionKeys, channel->mNumPositionKeys, ticks);
        // rotation matrix
        glm::mat4 rotation_matrix = interpolate_rotation(channel->mRotationKeys, channel->mNumRotationKeys, ticks);
        // scaling matrix
        glm::mat4 scaling_matrix = interpolate_scaling(channel->mScalingKeys, channel->mNumScalingKeys, ticks);
        
        current_transform = translation_matrix * rotation_matrix * scaling_matrix;
    } else {
        current_transform = assimp_to_glm_mat4(node->mTransformation);
    }
    if (bone_map.count(node_name)) {
        GLuint i = bone_map[node_name];
        bone_matrices[i] = transform * current_transform * bone_offsets[i];
    }
    for (int i = 0; i < node->mNumChildren; i++) {
        update_bone_matrices(animation_id, node->mChildren[i], transform * current_transform, ticks);
    }
}

void Model::draw(GLuint animation_id, const Shader& shader, double time){
    auto anim_length = scene->mAnimations[animation_id]->mDuration;
    auto anim_time = time * scene->mAnimations[animation_id]->mTicksPerSecond;
    double z = std::fmod(anim_time,anim_length);
    update_bone_matrices(animation_id, scene->mRootNode, glm::mat4{ 1.0f }, z);
    shader.use();
    shader.set_uniform_m4("u_M", glm::mat4{ 1.0f });
    shader.set_uniform_m4s("u_bones", bone_matrices);
    
    for (const auto& mesh: meshes) {
        mesh->draw(shader);
    }
}

void Model::draw(const Shader& shader){
    std::fill(bone_matrices.begin(), bone_matrices.end(), glm::mat4(1));
    shader.use();
    shader.set_uniform_m4("u_M", glm::mat4{ 1.0f });
    shader.set_uniform_m4s("u_bones", bone_matrices);
    
    for (const auto& mesh: meshes) {
        mesh->draw(shader);
    }
}
