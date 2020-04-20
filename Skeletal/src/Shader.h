#ifndef SKELETAL_SHADER_H
#define SKELETAL_SHADER_H

#include <string>

#include <filesystem>

class Shader {
public:
    Shader() = delete;
    Shader(const std::filesystem::path& vert_path, const std::filesystem::path& frag_path);

    inline void use() const { glUseProgram(ID); };

    void set_uniform_f(const std::string& name, const GLfloat& f) const;
    void set_uniform_i(const std::string& name, const GLuint& i) const;
    void set_uniform_v3(const std::string& name, const glm::vec3& vec3) const;
    void set_uniform_m4(const std::string& name, const glm::mat4& mat4) const;
    void set_uniform_m4s(const std::string& name, const std::vector<glm::mat4>& matrices) const;

private:
    static std::string read_file(const std::filesystem::path& path);
    static GLuint compile(GLuint type, const std::string& source);
    static void link(GLuint program, GLuint vertex, GLuint fragment);
    
    const GLuint ID;
};

#endif //SKELETAL_SHADER_H