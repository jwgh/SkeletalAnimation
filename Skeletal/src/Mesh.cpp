#include <algorithm>

#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>

#include "Mesh.h"
#include "TextureManager.h"
#include "Util.h"

Mesh::Mesh(const std::filesystem::path& directory_path, aiMesh* mesh, const aiScene* scene,
           std::map<std::string, GLuint>& bone_map, GLuint& total_bones, std::vector<glm::mat4>& bone_offsets) {

    std::vector<Vertex> vertices;
    std::vector<GLuint> indices;

    auto material = scene->mMaterials[mesh->mMaterialIndex];
    aiString texture_file;
    material->Get(AI_MATKEY_TEXTURE(aiTextureType_DIFFUSE, 0), texture_file);
    if(auto texture = scene->GetEmbeddedTexture(texture_file.C_Str())) {
        //returned pointer is not null, read texture from memory
        texID = TextureManager::load_texture_from_memory(texture);
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
        std::filesystem::path path = directory_path;
        path /= item;
        texID = TextureManager::load_texture_from_file(path);
    }
    
    for (int i = 0; i < mesh->mNumVertices; i++) {
        Vertex v;
        v.position = glm::vec3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
        v.tex_coord = glm::vec2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y);
        v.normal = glm::vec3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);
        vertices.push_back(v);
    }
    for (auto i{ 0 }; i < mesh->mNumFaces; i++) {
        auto face = mesh->mFaces[i];
        for (auto j{ 0 }; j < face.mNumIndices; j++){
            indices.push_back(face.mIndices[j]);
        }
    }
    num_indices = indices.size();
    num_vertices = vertices.size();

    
    for (auto i{ 0 }; i < mesh->mNumBones; i++) {
        auto bone = mesh->mBones[i];
        auto id = bone_map.count(bone->mName.C_Str()) ?
                  bone_map[bone->mName.C_Str()] :
                  bone_map[bone->mName.C_Str()] = total_bones++;
        
        bone_offsets.resize(std::max(id + 1, (GLuint) bone_offsets.size()));
        bone_offsets[id] = assimp_to_glm_mat4(bone->mOffsetMatrix);
        
        for (auto j{ 0 }; j < bone->mNumWeights; j++) {
            auto weight = bone->mWeights[j];
            add_bone(vertices[weight.mVertexId], id, weight.mWeight);
        }
    }
        
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
}

Mesh::~Mesh() {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteTextures(1, &texID);
}

void Mesh::draw(const Shader& shader) const {
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texID);
    shader.set_uniform_i("u_diffuse0", 0); // texID?
    
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
