#version 410 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D u_gPos;
uniform sampler2D u_gNormal;
uniform sampler2D u_gAlbedoSpec;

struct Light {
    vec3 Position;
    vec3 Color;

    float Linear;
    float Quadratic;
};
const int NR_LIGHTS = 32;
uniform Light u_lights[NR_LIGHTS];
uniform vec3 u_viewPos;

void main(){
    // retrieve data from gbuffer
    vec3 FragPos = texture(u_gPos, TexCoords).rgb;
    vec3 Normal = texture(u_gNormal, TexCoords).rgb;
    vec3 Diffuse = texture(u_gAlbedoSpec, TexCoords).rgb;
    float Specular = texture(u_gAlbedoSpec, TexCoords).a;

    // then calculate lighting as usual
    vec3 lighting  = Diffuse * 0.6; // hard-coded ambient component
    vec3 viewDir  = normalize(u_viewPos - FragPos);
    for(int i = 0; i < NR_LIGHTS; ++i)
    {
        // diffuse
        vec3 lightDir = normalize(u_lights[i].Position - FragPos);
        vec3 diffuse = max(dot(Normal, lightDir), 0.0) * Diffuse * u_lights[i].Color;
        // specular
        vec3 halfwayDir = normalize(lightDir + viewDir);
        float spec = pow(max(dot(Normal, halfwayDir), 0.0), 16.0);
        vec3 specular = u_lights[i].Color * spec * Specular;
        // attenuation
        float distance = length(u_lights[i].Position - FragPos);
        float attenuation = 1.0 / (1.0 + u_lights[i].Linear * distance + u_lights[i].Quadratic * distance * distance);
        diffuse *= attenuation;
        specular *= attenuation;
        lighting += diffuse + specular;
    }
    FragColor = vec4(lighting, 1.0);
}