#version 410 core

in vec3 v_worldPosition;
in vec2 v_UV;
in vec3 v_normal;

struct Material{
    sampler2D diffuse;
    sampler2D specular;
    float shininess;
};

struct Light_Dir{
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    vec3 direction;
};
struct Light_Point{
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    vec3 position;
    float K_c;
    float K_l;
    float K_q;
};

uniform sampler2D u_diffuse0;
uniform vec3 u_cameraPos;
uniform Material u_material;
uniform Light_Dir u_light_sun;
uniform Light_Point u_light_point;

out vec4 fragColor;

void main() {
    vec3 diffuse_map = vec3(texture(u_material.diffuse, v_UV));
    vec3 light_pos = u_light_point.position;

    float distance    = length(light_pos - v_worldPosition);
    float attenuation = 1.0 / (u_light_point.K_c + u_light_point.K_l * distance +
    u_light_point.K_q * (distance * distance));

    vec3 ambient =  u_light_point.ambient * diffuse_map;

    vec3 norm = normalize(v_normal);
    vec3 light_dir = normalize(light_pos - v_worldPosition);
    //vec3 light_dir = normalize(-u_light_sun.direction);
    float diff = max(dot(norm, light_dir), 0.0);
    vec3 diffuse = u_light_point.diffuse * diff * diffuse_map;

    vec3 view_dir = normalize(u_cameraPos - v_worldPosition);
    vec3 halfway_dir = normalize(light_dir + view_dir);
    vec3 reflect_dir = reflect(-light_dir, norm);
    float spec = pow(max(dot(norm, halfway_dir), 0.0), u_material.shininess);
    vec3 specular = u_light_point.specular * spec * vec3(texture(u_material.specular, v_UV));

    ambient  *= attenuation;
    diffuse  *= attenuation;
    specular *= attenuation;

    vec3 result_light = ambient + diffuse + specular;
    fragColor = vec4(result_light, 1.0f);
}
