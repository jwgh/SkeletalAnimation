#version 410 core

layout (location = 0) in vec3 a_position;
layout (location = 1) in vec2 a_UV;
layout (location = 2) in vec3 a_normal;
layout (location = 3) in ivec4 a_bones0; // the fingers get weird with 4 or less bone influences / vertex...
layout (location = 4) in ivec4 a_bones1; // ... so 2 vectors of 4 are used instead. Now 8 bones can influence a vertex.
layout (location = 5) in vec4 a_boneWeights0;
layout (location = 6) in vec4 a_boneWeights1;

out vec3 v_worldPosition;
out vec2 v_UV;
out vec3 v_normal;

uniform mat4 u_M;
uniform mat4 u_V;
uniform mat4 u_P;
uniform mat4 u_bones[100];

void main() {
    mat4 B = mat4(0.0f);
    for (int i = 0; i < 4; i++) {
        B += u_bones[a_bones0[i]] * a_boneWeights0[i];
        B += u_bones[a_bones1[i]] * a_boneWeights1[i];
    }
    gl_Position = u_P * u_V * u_M * B * vec4(a_position, 1);
    v_worldPosition = vec3(u_M * B * vec4(a_position, 1));
    v_UV = a_UV;
    v_normal = vec3(transpose(inverse(u_M * B)) * vec4(a_position, 0)); // inverse is expensive and should probably be done on CPU
}
