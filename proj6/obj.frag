#version 420 core

struct outdata{
    vec3 world_pos;
    vec3 eye_pos;
    vec2 texcoord;
    vec3 world_normal;
};

layout(std140, binding = 1) uniform DirectionalLightBlock 
{
    vec3 directional_dir;
    vec3 directional_radiance;
} directional;

layout(std140, binding = 2) uniform MaterialBlock 
{
    vec3 ambient;
    vec3 albedo;
    vec3 specular;
    float shininess;
} material;

in outdata o;
out vec4 color;

void main(){
    vec3 normal = normalize(o.world_normal);
    vec3 eye_dir = normalize(o.eye_pos - o.world_pos);
    vec3 h = normalize(directional.directional_dir + eye_dir);
    vec3 diffuse = max(dot(normal, directional.directional_dir), 0) * directional.directional_radiance * material.albedo;
    vec3 specular = vec3(max(dot(normal, h), 0)); //pow(max(dot(normal, h), 0), material.shininess) * directional.directional_radiance * material.specular;
    color = vec4(specular + material.ambient, 1);
}