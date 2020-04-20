//
// Created by Joakim Wingård on 2020-04-19.
//

#ifndef SKELETAL_UTIL_H
#define SKELETAL_UTIL_H

#include <assimp/scene.h>
#include <glm/glm.hpp>

inline static const double PI{ 3.14159265359 };

inline static glm::mat4 assimp_to_glm_mat4(const aiMatrix4x4& ai_mat){
    glm::mat4 glm_mat{ 0.0f };
    for (auto row{0}; row < 4; row++){
        for (auto col{0}; col < 4; col++){
            glm_mat[col][row] = ai_mat[row][col];
        }
    }
    return glm_mat;
}

inline static glm::mat4 assimp_quat_to_glm_mat4(const aiQuaternion& quat){
    auto rotation_matrix = quat.GetMatrix();
    glm::mat4 res(1.0f);
    for (auto row{0}; row < 3; row++){
        for(auto col{0}; col < 3; col++){
            res[col][row] = rotation_matrix[row][col];
        }
    }
    return res;
}

#endif //SKELETAL_UTIL_H
