#version 410 core

in vec3 v_worldPosition;
in vec2 v_UV;
in vec3 v_normal;
in vec3 v_tangent;
in vec3 v_bitangent;
in mat3 v_TBN;

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

#define NR_POINT_LIGHTS 4
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
uniform sampler2D u_normalMap0;
uniform vec3 u_cameraPos;
uniform Material u_material;
uniform Light_Dir u_light_sun;
uniform Light_Point u_light_point[NR_POINT_LIGHTS];


out vec4 fragColor;

vec3 calculate_directional(Light_Dir light, vec3 normal, vec3 view_dir){
    vec3 light_dir = normalize(-light.direction);
    float diff = max(dot(normal, light_dir), 0.0);

    //vec3 reflect_dir = reflect(-light_dir, normal);
    vec3 halfway_dir = normalize(light_dir + view_dir);
    float spec = pow(max(dot(normal, halfway_dir), 0.0), u_material.shininess);
    // combine results
    vec3 diff_texture = vec3(texture(u_material.diffuse, v_UV));
    vec3 ambient  = light.ambient  * diff_texture;
    vec3 diffuse  = light.diffuse  * diff * diff_texture;
    vec3 specular = light.specular * spec * vec3(texture(u_material.specular, v_UV));
    return ambient + diffuse + specular;
}

vec3 calculate_point_light(Light_Point light, vec3 normal, vec3 world_pos, vec3 view_dir){
    vec3 light_dir = normalize(light.position - world_pos);
    // diffuse shading
    float diff = max(dot(normal, light_dir), 0.0);
    // specular shading
    //vec3 reflectDir = reflect(-light_dir, normal);
    vec3 halfway_dir = normalize(light_dir + view_dir);
    float spec = pow(max(dot(normal, halfway_dir), 0.0), u_material.shininess);
    // attenuation
    float distance    = length(light.position - world_pos);
    float attenuation = 1.0 / (light.K_c + light.K_l * distance +
    light.K_q * (distance * distance));
    // combine results
    vec3 diff_texture = vec3(texture(u_material.diffuse, v_UV));
    vec3 ambient  = light.ambient  * diff_texture;
    vec3 diffuse  = light.diffuse  * diff * diff_texture;
    vec3 specular = light.specular * spec * vec3(texture(u_material.specular, v_UV));
    ambient  *= attenuation;
    diffuse  *= attenuation;
    specular *= attenuation;
    return ambient + diffuse + specular;
}


void main() {
    vec3 norm = texture(u_normalMap0, v_UV).rgb;
    norm = normalize(norm * 2.0f -1.0f);


    vec3 norm_from_mesh = normalize(v_normal);
    vec3 view_dir = normalize(u_cameraPos - v_worldPosition);
    // phase 1: Directional lighting
    vec3 result_light = calculate_directional(u_light_sun, norm, view_dir);
    // phase 2: Point lights
    for(int i = 0; i < 1; i++){
        result_light += calculate_point_light(u_light_point[i], norm, v_worldPosition, view_dir);
    }
    // phase 3: Spot light
    //result += CalcSpotLight(spotLight, norm, FragPos, viewDir);

    fragColor = vec4(result_light, 1.0f);
}
