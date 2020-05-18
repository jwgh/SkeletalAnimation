#version 410 core
out vec4 fragColor;

struct Light {
//vec3 position;
    vec3 direction;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

in vec2 v_UV;
in float v_height;
in vec3 v_worldPos;
in vec3 v_normal;


uniform vec3 u_viewPos;
uniform Light u_light;

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

    vec3 ambient = u_light.ambient * color;

    vec3 norm = normalize(v_normal);
    vec3 lightDir = normalize(-u_light.direction);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = u_light.diffuse * diff * color;

    vec3 viewDir = normalize(u_viewPos - v_worldPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 4.0f);
    vec3 specular = u_light.specular * spec * color;

    vec3 result = ambient + diffuse + specular;
    fragColor = vec4(result, 1.0f);
}