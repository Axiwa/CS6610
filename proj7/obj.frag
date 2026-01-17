#version 420 core

struct outdata{
    vec3 world_pos;
    vec3 eye_pos;
    vec2 texcoord;
    vec3 world_normal;
};

layout(std140, binding = 1) uniform LightBlock 
{
    vec3 pos;
    vec3 direction;
    vec3 radiance;
    vec3 angle;
    float padding;
    int type;
} light;

layout(std140, binding = 2) uniform MaterialBlock 
{
    vec3 ambient;
    vec3 albedo;
    vec3 specular;
    float padding;
    float shininess;
} material;

in outdata o;
in vec4 posinlightspace;
out vec4 color;

uniform sampler2DShadow shadow;

void main(){
    vec3 normal = normalize(o.world_normal);
    vec3 eyedir = normalize(o.eye_pos - o.world_pos);

    vec3 posinlight = posinlightspace.xyz / posinlightspace.w;
    posinlight = posinlight * 0.5 + 0.5;
    
    vec3 lightdir = vec3(0);
    vec3 radiance = vec3(0);
    if (light.type == 0){
        lightdir= normalize(-light.direction);
        radiance = light.radiance;
    }
    else if (light.type == 1){
        lightdir= normalize(light.pos - o.world_pos);
        float lightdist = max(length(light.pos - o.world_pos), 0.001);

        if (cos(light.angle.x) > dot(-lightdir, normalize(light.direction))) // outside cone
        {
            radiance = vec3(0);
        }
        else{
            radiance = light.radiance / lightdist / lightdist;
        }
    }

    radiance *= texture(shadow, posinlight);

    vec3 h = normalize(lightdir + eyedir);

    vec3 ambient = material.ambient;
    vec3 diffuse = (1-ambient) * max(dot(lightdir, normal), 0) * radiance * material.albedo;
    vec3 specular = (1-ambient) * pow(max(dot(h, normal), 0), material.shininess) * radiance * material.specular;

    color = vec4(diffuse + specular, 1);
}