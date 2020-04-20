#include <iostream>
#include <vector>
#include <fstream>

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Shader.h"

Shader::Shader(const std::filesystem::path& vert_path, const std::filesystem::path& frag_path)
    : ID{ glCreateProgram() } {
    std::string vert_src = read_file(vert_path);
    std::string frag_src = read_file(frag_path);
    GLuint vs_id = compile(GL_VERTEX_SHADER, vert_src);
    GLuint fs_id = compile(GL_FRAGMENT_SHADER, frag_src);
    link(ID, vs_id, fs_id);
}

GLuint Shader::compile(GLuint type, const std::string& source) {
    GLuint shader_id{ glCreateShader(type) };
    const char* temp = source.c_str();
    glShaderSource(shader_id, 1, &temp, nullptr);
    glCompileShader(shader_id);
    int success;
    glGetShaderiv(shader_id, GL_COMPILE_STATUS, &success);
    if (success){
        return shader_id;
    }
    int err_len;
    glGetShaderiv(shader_id, GL_INFO_LOG_LENGTH, &err_len);
    char error[err_len+1];
    glGetShaderInfoLog(shader_id, err_len, nullptr, error);
    std::cout << "Failed to compile shader, Error: " << error << std::endl;
    return -1;
}

void Shader::link(GLuint program, GLuint vertex, GLuint fragment) {
    glAttachShader(program, vertex);
    glAttachShader(program, fragment);

    glLinkProgram(program);

    glDeleteShader(vertex);
    glDeleteShader(fragment);

    int success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (success){
        return;
    }
    int err_len;
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &err_len);
    char error[err_len+1];
    glGetProgramInfoLog(program, err_len, nullptr, error);
    std::cout << "Unable to link shader, Error: " << error << std::endl;
}

void Shader::set_uniform_i(const std::string& name, const GLuint& i) const {
    GLint location{ glGetUniformLocation(ID, name.c_str()) };
    glUniform1i(location, i);
}

void Shader::set_uniform_f(const std::string& name, const GLfloat& f) const {
    GLint location{ glGetUniformLocation(ID, name.c_str()) };
    glUniform1f(location, f);
}

void Shader::set_uniform_v3(const std::string& name, const glm::vec3& vec3) const {
    GLint location{ glGetUniformLocation(ID, name.c_str())} ;
    glUniform3fv(location, 1, value_ptr(vec3));
}

void Shader::set_uniform_m4(const std::string& name, const glm::mat4& mat4) const {
    GLint location{ glGetUniformLocation(ID, name.c_str()) };
    glUniformMatrix4fv(location, 1, GL_FALSE, value_ptr(mat4));
}

void Shader::set_uniform_m4s(const std::string& name, const std::vector<glm::mat4>& matrices) const {
    GLint location{ glGetUniformLocation(ID, name.c_str()) };
    glUniformMatrix4fv(location, matrices.size(), GL_FALSE, value_ptr(matrices[0]));
}

std::string Shader::read_file(const std::filesystem::path& path){
    std::ifstream ifs(path.c_str());
    std::string res(
            (std::istreambuf_iterator<char>(ifs)),
            std::istreambuf_iterator<char>() );
    return std::move(res);
};
