

#include <map>
#include <iostream>
#include <string>
#include <vector>

#include <glad/glad.h>
#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#endif
#include "stb_image.h"

#include "TextureManager.h"

GLuint TextureManager::load_texture_from_file(const std::filesystem::path& path) {
    GLuint texture;
    static std::map<std::string, GLuint> loaded_textures;
    if (loaded_textures.count(path.c_str())) {
        return loaded_textures[path.c_str()];
    }

    int w, h, num_components;
    unsigned char* image_data = stbi_load(path.c_str(), &w, &h, &num_components, 0);
    if (image_data == nullptr) {
        std::cout << "Unable to load texture: " << path.c_str() << std::endl;
    }
    
    glGenTextures(1, &texture);
    
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

    switch(num_components){
        case 3:
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, image_data);
            break;
        case 4:
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, image_data);
            break;
        default:
            // unknown format
            break;
    }

    glBindTexture(GL_TEXTURE_2D, 0);
    
    stbi_image_free(image_data);
    return loaded_textures[path.c_str()] = texture;
}

GLuint TextureManager::load_texture_from_memory(const aiTexture* in_texture){
    GLuint new_texture;
    static std::map<std::string, GLuint> loaded_textures;
    if (loaded_textures.count(in_texture->mFilename.C_Str())) {
        return loaded_textures[in_texture->mFilename.C_Str()];
    }

    unsigned char* image_data{ nullptr };
    int w, h, num_components;
    image_data = stbi_load_from_memory(reinterpret_cast<unsigned char*>(in_texture->pcData), in_texture->mWidth, &w, &h, &num_components, 0);
    switch(num_components){
        case 3:
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, image_data);
            break;
        case 4:
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, image_data);
            break;
        default:
            // unknown format
            break;
    }
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

    glBindTexture(GL_TEXTURE_2D, 0);
    return new_texture;
}
