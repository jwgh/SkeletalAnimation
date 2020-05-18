#version 410 core
layout (location = 0) in vec3 a_pos;
layout (location = 1) in vec2 a_UV;
layout (location = 2) in vec3 a_normal;

out vec2 v_UV;

void main(){
    gl_Position = vec4(a_pos, 1.0);
    v_UV = a_UV;
}