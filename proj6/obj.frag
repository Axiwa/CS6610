#version 420 core

layout(std140, binding = 1) uniform DirectionalLightBlock 
{
    vec3 directional_dir;
    vec3 directional_radiance;
} directional;

layout(std140, binding = 2) uniform MaterialBlock 
{
    vec3 ambient;
    vec3 abledo;
    vec3 specular;
    float shininess;
} material;

out vec4 color;

void main(){
    color = vec4(1, 0, 1, 1);
}