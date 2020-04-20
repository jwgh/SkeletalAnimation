#include <iostream>

#include <glad/glad.h>
#include <assimp/cimport.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <glm/gtc/matrix_transform.hpp>

#include "Model.h"
#include "Util.h"
#include "TextureManager.h"

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
        meshes.emplace_back(std::make_shared<Mesh>());
        auto& new_mesh = *meshes.back();


        std::vector<Vertex> vertices;
        std::vector<GLuint> indices;

        auto material = scene->mMaterials[mesh->mMaterialIndex];
        aiString texture_file;
        material->Get(AI_MATKEY_TEXTURE(aiTextureType_DIFFUSE, 0), texture_file);
        if(auto texture = scene->GetEmbeddedTexture(texture_file.C_Str())) {
            //returned pointer is not null, read texture from memory
            new_mesh.diffuse0_ID = TextureManager::load_texture_from_memory(texture);
        } else {
            //regular file, check if it exists and read it
            material->GetTexture(aiTextureType_DIFFUSE, 0, &texture_file);
            std::string item = texture_file.C_Str();
            int i = item.length() - 1;
            for (; i >= 0; i--){
                if(item[i] == '\\'){
                    break;
                }
            }
            item = item.substr(i + 1);
            std::filesystem::path path = dir_path;
            path /= item;
            new_mesh.diffuse0_ID = TextureManager::load_texture_from_file(path);
        }

        for (auto v_idx{ 0 }; v_idx < mesh->mNumVertices; v_idx++) {
            Vertex v;
            v.position = glm::vec3(mesh->mVertices[v_idx].x, mesh->mVertices[v_idx].y, mesh->mVertices[v_idx].z);
            v.tex_coord = glm::vec2(mesh->mTextureCoords[0][v_idx].x, mesh->mTextureCoords[0][v_idx].y);
            v.normal = glm::vec3(mesh->mNormals[v_idx].x, mesh->mNormals[v_idx].y, mesh->mNormals[v_idx].z);
            vertices.push_back(v);
        }
        for (auto f_index{ 0 }; f_index < mesh->mNumFaces; f_index++) {
            auto face = mesh->mFaces[f_index];
            for (auto i_index{ 0 }; i_index < face.mNumIndices; i_index++){
                indices.push_back(face.mIndices[i_index]);
            }
        }
        new_mesh.num_indices = indices.size();
        new_mesh.num_vertices = vertices.size();


        for (auto b_index{ 0 }; b_index < mesh->mNumBones; b_index++) {
            auto bone = mesh->mBones[b_index];
            auto id = bone_map.count(bone->mName.C_Str()) ?
                      bone_map[bone->mName.C_Str()] :
                    bone_map[bone->mName.C_Str()] = total_bones++;

            bone_offsets.resize(std::max(id + 1, (GLuint) bone_offsets.size()));
            bone_offsets[id] = assimp_to_glm_mat4(bone->mOffsetMatrix);

            for (auto w_index{ 0 }; w_index < bone->mNumWeights; w_index++) {
                auto weight = bone->mWeights[w_index];
                new_mesh.add_bone(vertices[weight.mVertexId], id, weight.mWeight);
            }
        }

        new_mesh.setup_VAO(vertices, indices);
    }

    for(auto a_index{ 0 }; a_index < scene->mNumAnimations; a_index++){
        auto ai_animation = scene->mAnimations[a_index];
        Animation new_animation;
        new_animation.name = ai_animation->mName.C_Str();
        new_animation.ID = a_index;
        new_animation.duration = ai_animation->mDuration;
        new_animation.ticks_per_second = ai_animation->mTicksPerSecond;
        animations[a_index] = new_animation;
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
    const Animation& a{ animations[animation_id] };
    auto anim_length = a.duration;
    auto anim_time = time * a.ticks_per_second;
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
