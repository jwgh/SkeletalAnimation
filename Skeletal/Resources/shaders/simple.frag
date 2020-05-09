#version 410 core

in vec3 v_worldPosition;
in vec2 v_UV;
in vec3 v_normal;


uniform sampler2D u_diffuse0;
uniform vec3 u_cameraPos;

out vec4 fragColor;

void main() {
    vec3 light_pos = vec3(0.0f, -10.0f, 0.0f);
    vec3 light_color = vec3(1.0f, 0.0f, 0.0f);
    float ambient_strength = 0.4f;
    vec3 ambient = ambient_strength * light_color;

    vec3 norm = normalize(v_normal);
    vec3 light_dir = normalize(light_pos - v_worldPosition);
    float diff = max(dot(norm, light_dir), 0.0);
    vec3 diffuse = diff * light_color;

    float specular_strength = 0.5f;
    vec3 view_dir = normalize(u_cameraPos - v_worldPosition);
    vec3 reflect_dir = reflect(-light_dir, norm);
    float spec = pow(max(dot(view_dir, reflect_dir), 0.0), 32);
    vec3 specular = specular_strength * spec * light_color;

    vec3 result_light = ambient + diffuse + specular;
    fragColor = vec4(result_light, 1.0f) * vec4(texture(u_diffuse0, v_UV).rgb, 1.0f);
}
