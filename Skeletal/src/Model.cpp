#include <iostream>

#include <glad/glad.h>
#include <assimp/cimport.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

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

    /** Get ANIMATION data from the scene... */
    for(auto a_index{ 0 }; a_index < scene->mNumAnimations; a_index++){
        const auto& ai_animation = scene->mAnimations[a_index];
        Animation new_animation;
        new_animation.name = ai_animation->mName.C_Str();
        new_animation.ID = a_index;
        new_animation.duration = ai_animation->mDuration;
        new_animation.ticks_per_second = ai_animation->mTicksPerSecond;
        new_animation.num_channels = ai_animation->mNumChannels;

        for(auto c_index{ 0 }; c_index < new_animation.num_channels; c_index++){
            const auto& ai_channel = ai_animation->mChannels[c_index];
            Channel new_channel;
            new_channel.ID = c_index;
            new_channel.node_name = ai_channel->mNodeName.C_Str();
            new_channel.num_keyframes_position = ai_channel->mNumPositionKeys;
            new_channel.num_keyframes_rotation = ai_channel->mNumRotationKeys;
            new_channel.num_keyframes_scaling = ai_channel->mNumScalingKeys;

            for(auto p_index{ 0 }; p_index < new_channel.num_keyframes_position; p_index++){
                const auto& ai_pos_key = ai_channel->mPositionKeys[p_index];
                KeyFramePos new_keyframe{ ai_pos_key.mTime,
                                          glm::vec3{ ai_pos_key.mValue.x, ai_pos_key.mValue.y, ai_pos_key.mValue.z } };
                new_channel.keyframes_position.push_back(new_keyframe);
            }

            for(auto r_index{ 0 }; r_index < new_channel.num_keyframes_rotation; r_index++){
                const auto& ai_rot_key = ai_channel->mRotationKeys[r_index];
                KeyFrameRot new_keyframe{ ai_rot_key.mTime,
                                          glm::quat{ ai_rot_key.mValue.w, ai_rot_key.mValue.x, ai_rot_key.mValue.y, ai_rot_key.mValue.z } }; // unsure about the ordering here
                new_channel.keyframes_rotation.push_back(new_keyframe);

            }

            for(auto s_index{ 0 }; s_index < new_channel.num_keyframes_scaling; s_index++){
                const auto& ai_scale_key = ai_channel->mScalingKeys[s_index];
                KeyFrameScale new_keyframe{ ai_scale_key.mTime,
                                            glm::vec3{ ai_scale_key.mValue.x, ai_scale_key.mValue.z, ai_scale_key.mValue.z } };

            }
            new_animation.channels.push_back(new_channel);
        }

        animations[a_index] = new_animation;
    }

    /** ...Then RECURSIVELY create the tree of nodes( MESHES are created with nodes ) */
    root = initNode(scene->mRootNode, std::make_shared<Node>(Node()));
    bone_matrices.resize(total_bones);
}

Model::~Model() {
    aiReleaseImport(scene);
}

std::shared_ptr<Node> Model::initNode(aiNode* ai_node, std::shared_ptr<Node> new_node) {
    /** Create a new node, this is the CURRENT node in the tree (to make a root, send in a NEW sharedPTR */
    new_node->name = ai_node->mName.C_Str();
    new_node->transform = assimp_to_glm_mat4(ai_node->mTransformation);
    // TODO: get parent and meta data and crap?

    /** For each MESH in this node... */
    for (auto i{ 0 }; i < ai_node->mNumMeshes; i++) {
        auto mesh = scene->mMeshes[ai_node->mMeshes[i]];
        meshes.emplace_back(std::make_shared<Mesh>());
        auto& new_mesh = *meshes.back();

        new_mesh.name = ai_node->mName.C_Str();
        new_mesh.default_transform = assimp_to_glm_mat4( ai_node->mTransformation );

        std::vector<Vertex> vertices;
        std::vector<GLuint> indices;

        /** ... Get MATERIAL info for the mesh... */
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
        material->Get(AI_MATKEY_TEXTURE(aiTextureType_NORMALS, 0), texture_file);
        if(auto texture = scene->GetEmbeddedTexture(texture_file.C_Str())) {
            //returned pointer is not null, read texture from memory
            new_mesh.normal0_ID = TextureManager::load_texture_from_memory(texture);
        } else {
            //regular file, check if it exists and read it
            material->GetTexture(aiTextureType_HEIGHT, 0, &texture_file); // is this right?
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
            new_mesh.normal0_ID = TextureManager::load_texture_from_file(path);
        }
        material->Get(AI_MATKEY_TEXTURE(aiTextureType_SPECULAR, 0), texture_file);
        if(auto texture = scene->GetEmbeddedTexture(texture_file.C_Str())) {
            //returned pointer is not null, read texture from memory
            new_mesh.specular0_ID = TextureManager::load_texture_from_memory(texture);
        } else {
            //regular file, check if it exists and read it
            material->GetTexture(aiTextureType_SPECULAR, 0, &texture_file);
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
            new_mesh.specular0_ID = TextureManager::load_texture_from_file(path);
        }


        /** ... Get VERTEX info for the mesh... */
        for (auto v_idx{ 0 }; v_idx < mesh->mNumVertices; v_idx++) {
            Vertex v;
            v.position = glm::vec3(mesh->mVertices[v_idx].x, mesh->mVertices[v_idx].y, mesh->mVertices[v_idx].z);
            v.tex_coord = glm::vec2(mesh->mTextureCoords[0][v_idx].x, mesh->mTextureCoords[0][v_idx].y);
            v.normal = glm::vec3(mesh->mNormals[v_idx].x, mesh->mNormals[v_idx].y, mesh->mNormals[v_idx].z);
            vertices.push_back(v);
        }

        /** ... Get INDICES info for the mesh... */
        for (auto f_index{ 0 }; f_index < mesh->mNumFaces; f_index++) {
            auto face = mesh->mFaces[f_index];
            for (auto i_index{ 0 }; i_index < face.mNumIndices; i_index++){
                indices.push_back(face.mIndices[i_index]);
            }
        }
        new_mesh.num_indices = indices.size();
        new_mesh.num_vertices = vertices.size();

        /** ... Get BONE info for the mesh... */
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

        /** ... FINALLY (for the mesh part), setup the VAO, VBO and EBO data and all the buffers on the GPU... */
        new_mesh.setup_VAO(vertices, indices);
    }

    /**
     * ...Recursively create the rest of the nodes...
     */
    for (auto i{ 0 }; i < ai_node->mNumChildren; i++) {
        new_node->children.emplace_back(initNode(ai_node->mChildren[i], std::make_shared<Node>(Node())));
    }
    return new_node;
}

void Model::update_bone_matrices(int animation_ID, std::shared_ptr<Node> node, const glm::mat4& transform, double ticks){
    const auto& node_name = node->name;


    auto& animation = animations[animation_ID];
    //const auto& node_name = animation2.name;
    glm::mat4 current_transform;
    if (anim_channels.count(std::tuple<GLuint, std::string>(animation_ID, node_name))) {
        GLuint channel_id = anim_channels[std::tuple<GLuint, std::string>(animation_ID, node_name)];

        const auto& channel = animation.channels[channel_id];

        glm::mat4 T = interpolate_translation(channel.keyframes_position, ticks);
        glm::mat4 R = interpolate_rotation(channel.keyframes_rotation, ticks);
        glm::mat4 S = interpolate_scaling(channel.keyframes_scaling, ticks);

        current_transform = T * R * S;
    } else {
        current_transform = node->transform;
    }
    if (bone_map.count(node_name)) {
        GLuint i = bone_map[node_name];
        bone_matrices[i] = transform * current_transform * bone_offsets[i];
    }
    for (const auto& child : node->children) {
        update_bone_matrices(animation_ID, child, transform * current_transform, ticks);
    }
}

void Model::draw(GLuint animation_id, const Shader& shader, double time){
    const Animation& a{ animations[animation_id] };
    auto anim_length = a.duration;
    auto anim_time = time * a.ticks_per_second;
    double z = std::fmod(anim_time,anim_length);
    update_bone_matrices(animation_id, root, glm::mat4{ 1.0f }, z);

    shader.set_uniform_m4s("u_bones", bone_matrices);
    
    for (const auto& mesh: meshes) {
        mesh->draw(shader);
    }
}


void Model::draw(const Shader& shader){
    std::fill(bone_matrices.begin(), bone_matrices.end(), glm::mat4(1));
    shader.set_uniform_m4s("u_bones", bone_matrices);
    
    for (const auto& mesh: meshes) {
        mesh->draw(shader);
    }
}

glm::mat4 Model::interpolate_translation(const std::vector<KeyFramePos>& keys, double ticks){
    if (keys.empty()) {
        return glm::mat4{ 1.0f };
    }
    if (keys.size() == 1 || ticks <= keys[0].time){
        return glm::translate( glm::mat4{ 1.0f }, keys[0].vec );
    }
    if (keys.back().time <= ticks){
        return glm::translate(glm::mat4{ 1.0f }, keys.back().vec);
    }

    KeyFramePos anchor;
    anchor.time = ticks;
    auto right_ptr = std::upper_bound(keys.begin(), keys.end(), anchor, [] (const KeyFramePos& a, const KeyFramePos& b) {
        return a.time < b.time;
    });
    auto left_ptr = right_ptr - 1;

    float factor = (ticks - left_ptr->time) / (right_ptr->time - left_ptr->time);

    static glm::vec3 interpolated = left_ptr->vec * (1.0f - factor) + right_ptr->vec * factor;
    return glm::translate(glm::mat4{ 1.0f }, interpolated);
}

glm::mat4 Model::interpolate_rotation(const std::vector<KeyFrameRot>& keys, double ticks){
    if (keys.empty()) {
        return glm::mat4{ 1.0f };
    }
    if (keys.size() == 1 || ticks <= keys[0].time){
        return glm::mat4_cast(keys[0].quat);
    }
    if (keys.back().time <= ticks) {
        return glm::mat4_cast(keys.back().quat);
    }

    KeyFrameRot anchor;
    anchor.time = ticks;
    auto right_ptr = std::upper_bound(keys.begin(), keys.end(), anchor, [] (const KeyFrameRot& a, const KeyFrameRot& b) {
        return a.time < b.time;
    });
    auto left_ptr = right_ptr - 1;

    float factor = (ticks - left_ptr->time) / (right_ptr->time - left_ptr->time);

    return glm::mat4_cast(glm::slerp(left_ptr->quat, right_ptr->quat, factor));
}

glm::mat4 Model::interpolate_scaling(const std::vector<KeyFrameScale>& keys, double ticks){
    if (keys.empty()) {
        return glm::mat4{ 1.0f };
    }
    if (keys.size() == 1 || ticks <= keys[0].time){
        return glm::scale( glm::mat4{ 1.0f }, keys[0].vec );
    }
    if (keys.back().time <= ticks){
        return glm::scale(glm::mat4{ 1.0f }, keys.back().vec );
    }

    KeyFrameScale anchor;
    anchor.time = ticks;
    auto right_ptr = std::upper_bound(keys.begin(), keys.end(), anchor, [] (const KeyFrameScale& a, const KeyFrameScale& b) {
        return a.time < b.time;
    });
    auto left_ptr = right_ptr - 1;

    float factor = (ticks - left_ptr->time) / (right_ptr->time - left_ptr->time);

    glm::vec3 interpolated = left_ptr->vec * (1.0f - factor) + right_ptr->vec * factor;
    return glm::scale(glm::mat4{ 1.0f }, interpolated);
}

