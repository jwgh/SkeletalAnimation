#version 410 core
layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec4 gAlbedoSpec;

in vec2 TexCoords;
in vec3 FragPos;
in vec3 Normal;

uniform sampler2D texture_diffuse1;
uniform sampler2D texture_specular1;
uniform sampler2D u_normalMap0;

struct Material{
    sampler2D diffuse;
    sampler2D specular;
    float shininess;
};
uniform Material u_material;

void main(){
    // store the fragment position vector in the first gbuffer texture
    gPosition = FragPos;
    // also store the per-fragment normals into the gbuffer
    //gNormal = normalize(texture(u_normalMap0, TexCoords).rgb);
    gNormal = normalize(Normal);

    // and the diffuse per-fragment color
    gAlbedoSpec.rgb = texture(u_material.diffuse, TexCoords).rgb;
    // store specular intensity in gAlbedoSpec's alpha component
    gAlbedoSpec.a = texture(u_material.specular, TexCoords).r;
}