#version 410 core

in vec3 v_worldPosition;
in vec2 v_UV;
in vec3 v_normal;


uniform sampler2D u_diffuse0;

out vec4 fragColor;

void main() {
    fragColor = vec4(texture(u_diffuse0, v_UV).rgb, 1);
}
