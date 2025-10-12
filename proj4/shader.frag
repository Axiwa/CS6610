#version 330 core

in vec3 out_normal;
in vec3 out_eye;

uniform vec3 in_albedo;
uniform float in_roughness;
uniform vec3 in_lightdir;
uniform vec3 in_radiance;

uniform bool hasTexture;

uniform sampler2D diffuse;
uniform sampler2D specular;
in vec2 out_texcoord;

out vec4 color;

void main(){
    vec3 normal = normalize(out_normal);
    vec3 eye = normalize(out_eye);
    vec3 lightdir = normalize(in_lightdir);
    vec3 halfv = normalize(lightdir + eye);

    vec3 albedo = vec3(0);
    vec3 specularcolor = vec3(0);
    float roughness = in_roughness;

    if (hasTexture){
        albedo = texture(diffuse, out_texcoord).xyz;
        specularcolor = texture(specular, out_texcoord).xyz;
    }
    else{
        albedo = in_albedo;
        specularcolor = albedo;
    }

    vec3 ambient = vec3(0.1f);
    vec3 diffuse = (1 - ambient) * in_radiance * albedo * max(0, dot(lightdir, normal));
    vec3 specular = (1 - ambient) * in_radiance * specularcolor * pow(max(0, dot(halfv, normal)), roughness);
    color = vec4(ambient + specular + diffuse, 1);
}