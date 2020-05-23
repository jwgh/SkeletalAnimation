#version 410 core

layout (location = 0) in vec3 a_pos;

out vec3 v_UV;

uniform mat4 u_P;
uniform mat4 u_V;
uniform mat4 u_M;

void main()
{
    v_UV = a_pos;
    vec4 pos = u_P * u_V * u_M * vec4(a_pos, 1.0);
    gl_Position = pos.xyww;
}