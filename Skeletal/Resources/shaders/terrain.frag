#version 410 core
out vec4 fragColor;

in vec2 v_UV;
in float v_height;

uniform sampler2D u_Diffuse0;

float remap( float old_max, float old_min, float new_max, float new_min, float current_value){
    return (old_min != old_max && new_min != new_max)
    ? (((current_value - old_min) * (new_max - new_min)) / (old_max - old_min)) + new_min
    : (new_max + new_min) / 2.0f;
}
const vec3 R = vec3( 1.0f, 0.0f, 0.0f );
const vec3 G = vec3( 0.0f, 1.0f, 0.0f );
const vec3 B = vec3( 0.0f, 0.0f, 1.0f );

void main(){
    float remapped_height = remap(12.0f, -12.0f, 1.0f, -1.0f, v_height);

    vec3 color = remapped_height > 0.0f
        ? mix(G, R, remap(1.0f, -1.0f, 0.5f, 0.0f, remapped_height))
        : mix(B, G, remap(1.0f, -1.0f, 1.0f, 0.5f, remapped_height));
    fragColor = vec4(color.xyz, 1.0f);
}