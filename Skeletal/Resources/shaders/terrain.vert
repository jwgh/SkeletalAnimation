#version 410 core
layout (location = 0) in vec3 a_pos;
layout (location = 1) in vec2 a_UV;
layout (location = 2) in vec3 a_normal;

out vec2 v_UV;

uniform mat4 u_M;
uniform mat4 u_V;
uniform mat4 u_P;


void main(){
    gl_Position = u_P * u_V * u_M * vec4(a_pos, 1.0f);
    v_UV = a_UV;
}