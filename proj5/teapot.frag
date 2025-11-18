#version 330 core 

in vec2 out_coord;
in vec3 world_pos;
in vec3 world_normal;

uniform vec3 in_eye;
uniform vec3 in_light;

uniform sampler2D basecolorTex;
uniform sampler2D specularcolorTex;

uniform vec3 albedo_color;
uniform vec3 specular_color;

out vec4 color;
void main(){
    vec3 ambient = vec3(0.1, 0.1, 0.1);
    vec3 albedo = albedo_color * texture(basecolorTex, out_coord).xyz;
    vec3 specular = specular_color * texture(specularcolorTex, out_coord).xyz;

    vec3 radiance = vec3(1, 1, 1);
    float shininess = 500;

    vec3 eye_dir = normalize(in_eye - world_pos);
    vec3 h = normalize(in_light + eye_dir);

    vec3 diffuse = (1-ambient) * radiance * albedo * max(0, dot(normalize(world_normal), in_light));
    vec3 glossy = (1-ambient) * radiance * specular * pow(max(0, dot(normalize(world_normal), h)), shininess);
    color = vec4(ambient + diffuse + glossy, 1);
}