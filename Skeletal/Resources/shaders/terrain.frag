#version 410 core
out vec4 fragColor;

in vec2 v_UV;

uniform sampler2D u_Diffuse0;

void main(){
    fragColor = texture(u_Diffuse0, v_UV);
}