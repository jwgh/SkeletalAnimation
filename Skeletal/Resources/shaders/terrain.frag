#version 410 core
out vec4 fragColor;

in vec2 v_UV;
in float v_height;

uniform sampler2D u_Diffuse0;

void main(){
    fragColor = vec4(normalize(vec3(v_height*0.5f, 0.5f, 0.5f)).xyz, 1.0f);
}