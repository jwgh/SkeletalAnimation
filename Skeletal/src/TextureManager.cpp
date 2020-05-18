

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

    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    int w, h, num_components;
    unsigned char* image_data = stbi_load(path.c_str(), &w, &h, &num_components, 0);
    if (image_data == nullptr) {
        std::cout << "Unable to load texture: " << path.c_str() << std::endl;
    }
    switch(num_components){
        case 3:
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, image_data);
            break;
        case 4:
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, image_data);
            break;
        default:
            // unknown format
            std::cout << "???" << std::endl;
            break;
    }

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);



    glBindTexture(GL_TEXTURE_2D, 0);
    
    stbi_image_free(image_data);
    loaded_textures[path.c_str()] = texture;
    std::cout << "from file: " << path.c_str() << ", " << texture << std::endl;
    return texture;
}

GLuint TextureManager::load_texture_from_memory(const aiTexture* in_texture){
    GLuint new_texture;
    static std::map<std::string, GLuint> loaded_textures;
    if (loaded_textures.count(in_texture->mFilename.C_Str())) {
        return loaded_textures[in_texture->mFilename.C_Str()];
    }

    glGenTextures(1, &new_texture);
    glBindTexture(GL_TEXTURE_2D, new_texture);

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
    std::cout << "from memory: " << in_texture->mFilename.C_Str() << ", " << new_texture << std::endl;
    return new_texture;
}

GLuint TextureManager::load_single_color_texture(unsigned char r, unsigned char g, unsigned char b){

    GLuint new_texture;

    glGenTextures(1, &new_texture);
    glBindTexture(GL_TEXTURE_2D, new_texture);
    GLubyte texData[4] = { r, g, b, 255 };

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, &texData[0]);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

    glBindTexture(GL_TEXTURE_2D, 0);
    return new_texture;
}
