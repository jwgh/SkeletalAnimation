#version 410 core
out vec4 frag_color;

in vec3 v_UV;

uniform samplerCube u_skybox;

void main(){
    frag_color = texture(u_skybox, v_UV);
}