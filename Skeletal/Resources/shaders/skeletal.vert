#version 410 core

layout (location = 0) in vec3 a_position;
layout (location = 1) in vec2 a_UV;
layout (location = 2) in vec3 a_normal;
layout (location = 3) in ivec4 a_bones0; // the fingers get weird with 4 or less bone influences / vertex...
layout (location = 4) in ivec4 a_bones1; // ... so 2 vectors of 4 are used instead. Now 8 bones can influence a vertex.
layout (location = 5) in vec4 a_boneWeights0;
layout (location = 6) in vec4 a_boneWeights1;
layout (location = 7) in vec3 a_tangent;

out vec3 v_worldPosition;
out vec2 v_UV;
out vec3 v_normal;
out vec3 v_tangent;

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
    /*
    I'm not 100% sure how to implement normal mapping with multiple lights
    Previously, I did all light calculations in world space, with a light struct uniform vector in frag shader
    But now I guess I need to transform all light positions and shit to tangent space with the TBN matrix in the vertex shader
    and use in/out to communicate this...

    Refactor idea:
    break the light struct, don't have position in the struct in the frag shader
    input a vector of light positions as a uniform to the v shader, same order als light struct vector in frag shader
    transform to tangent space, send transformed vectors via out/in

    refactor fragment shader so that it uses the tangent space calculations

    how TF am I supposed to handle directional light?
    */
    /*
    mat3 normal_matrix = transpose(inverse(mat3(u_M * B)));
    vec3 tangent = normalize(normal_matrix * a_tangent);
    vec3 normal = normalize(normal_matrix * a_normal);
    tangent = normalize(tangent - dot(tangent, normal) * normal);
    vec3 bitangent = cross(normal, tangent);

    mat3 TBN = transpose(mat3(tangent, bitangent, normal));

    //vs_out.TangentLightPos = TBN * lightPos;
    vs_out.TangentViewPos  = TBN * viewPos;
    vs_out.TangentFragPos  = TBN * vs_out.FragPos;
    */
    v_normal = vec3(transpose(inverse(u_M * B)) * vec4(a_position, 0)); // inverse is expensive and should probably be done on CPU
}
