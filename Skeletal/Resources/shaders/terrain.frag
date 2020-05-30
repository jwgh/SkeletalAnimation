#version 410 core
out vec4 fragColor;

struct Light {
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
uniform sampler2D u_Diffuse1;
uniform sampler2D u_Diffuse2;
uniform sampler2D u_Diffuse3;
uniform sampler2D u_Diffuse4;

float remap( float old_max, float old_min, float new_max, float new_min, float current_value){
    return (old_min != old_max && new_min != new_max)
    ? (((current_value - old_min) * (new_max - new_min)) / (old_max - old_min)) + new_min
    : (new_max + new_min) / 2.0f;
}
const vec3 W = vec3( 1.0f );
const vec3 R = vec3( 0.5f, 0.5f, 0.5f );
const vec3 G = vec3( 0.0f, 1.0f, 0.0f );
const vec3 B = vec3( 0.0f, 0.0f, 1.0f );

void main(){
    vec4 snow, rock, grass, dirt, sand;
    snow = texture(u_Diffuse0, v_UV);
    rock = texture(u_Diffuse1, v_UV);
    grass = texture(u_Diffuse2, v_UV);
    dirt = texture(u_Diffuse3, v_UV);
    sand = texture(u_Diffuse4, v_UV);

    vec4 color = vec4(1.0f);
    color = v_height > 36.0f ? snow :
    (v_height <= 36 && v_height > 32) ? (mix(snow, rock, smoothstep(36, 32, v_height))) :
    (v_height <= 32 && v_height > 16) ? rock :
    (v_height <= 16 && v_height > 12) ? (mix(rock, grass, smoothstep(16, 12, v_height))) :
    (v_height <= 12 && v_height > 2) ? grass :
    (v_height <= 2 && v_height > -1) ? (mix(grass, dirt, smoothstep(2, -1, v_height))) :
    (v_height <= -1 && v_height > -12) ? dirt :
    (v_height <= -12 && v_height > -14) ? (mix(dirt, sand, smoothstep(-12, -14, v_height))) : sand;


    //vec4 color = mix(snow, rock, smoothstep(16.0f, 14.0f, v_height));
    //color = mix(color, grass, smoothstep(14.0f, 8.0f, v_height));
    //color = mix(color, dirt, smoothstep(8.0f, 0.0f, v_height));
    //color = mix(color, sand, smoothstep(0.0f, -8.0f, v_height));
    //vec3 color = mix(W, R, smoothstep(16.0f, 12.0f, v_height));
    //color = mix(R, G, smoothstep(12.0f, 0.0f, v_height));
    //color = mix(color, B, smoothstep(0.0f, -16.0f, v_height));

    vec3 ambient = u_light.ambient * color.xyz;

    vec3 norm = normalize(v_normal);
    vec3 lightDir = normalize(-u_light.direction);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = u_light.diffuse * diff * color.xyz;

    vec3 viewDir = normalize(u_viewPos - v_worldPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 4.0f);
    vec3 specular = u_light.specular * spec * color.xyz;

    vec3 result = ambient + diffuse + specular;
    fragColor = vec4(result, 1.0f);
}