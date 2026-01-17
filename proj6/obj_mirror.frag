#version 420 core

struct outdata{
    vec3 world_pos;
    vec3 eye_pos;
    vec2 texcoord;
    vec3 world_normal;
    float draw;
};

layout(std140, binding = 1) uniform DirectionalLightBlock 
{
    vec3 directional_dir;
    vec3 directional_radiance;
} directional;

layout(std140, binding = 2) uniform MaterialBlock 
{
    vec3 ambient;
    float padding1;
    vec3 albedo;
    float padding2;
    vec3 specular;
    float padding3;
    float shininess;
} material;

uniform samplerCube envTex;

in outdata o;
out vec4 color;

void main(){
    vec3 normal = normalize(o.world_normal);
    vec3 eye_dir = normalize(o.eye_pos - o.world_pos);
    vec3 env_dir = reflect(-eye_dir, normal);
    vec3 h = normalize(directional.directional_dir + eye_dir);
    vec3 diffuse = (1 - material.ambient) * max(dot(normal, directional.directional_dir), 0) * directional.directional_radiance * material.albedo;
    vec3 glossy = (1 - material.ambient) * pow(max(dot(normal, h), 0), material.shininess) * directional.directional_radiance * material.specular;
    vec3 specular = (1 - material.ambient) * texture(envTex, env_dir).xyz;
    color = vec4((material.ambient + glossy + specular) * o.draw, 1 * o.draw);
}