#ifndef SKELETAL_TEXTUREMANAGER_H
#define SKELETAL_TEXTUREMANAGER_H

#include <filesystem>
#include <assimp/texture.h>

class TextureManager {
public:
    static GLuint load_texture_from_file(const std::filesystem::path& path);
    static GLuint load_texture_from_memory(const aiTexture* in_texture);
    static GLuint load_single_color_texture(unsigned char r, unsigned char g, unsigned char b);

};

#endif //SKELETAL_TEXTUREMANAGER_H