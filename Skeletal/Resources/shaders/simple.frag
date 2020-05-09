#version 410 core

in vec3 v_worldPosition;
in vec2 v_UV;
in vec3 v_normal;

struct Material{
    sampler2D diffuse;
    sampler2D specular;
    float shininess;
};

struct Light{
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    vec3 position;
};

uniform sampler2D u_diffuse0;
uniform vec3 u_cameraPos;
uniform Material u_material;
uniform Light u_light;

out vec4 fragColor;

void main() {
    vec3 diffuse_map = vec3(texture(u_material.diffuse, v_UV));
    vec3 light_pos = u_light.position;
    vec3 ambient =  u_light.ambient * diffuse_map;

    vec3 norm = normalize(v_normal);
    vec3 light_dir = normalize(light_pos - v_worldPosition);
    float diff = max(dot(norm, light_dir), 0.0);
    vec3 diffuse = u_light.diffuse * diff * diffuse_map;

    float specular_strength = 0.5f;
    vec3 view_dir = normalize(u_cameraPos - v_worldPosition);
    vec3 halfway_dir = normalize(light_dir + view_dir);
    vec3 reflect_dir = reflect(-light_dir, norm);

    float spec = pow(max(dot(norm, halfway_dir), 0.0), u_material.shininess);
    vec3 specular = u_light.specular * spec * vec3(texture(u_material.specular, v_UV));

    vec3 result_light = ambient + diffuse + specular;
    fragColor = vec4(result_light, 1.0f);
}
