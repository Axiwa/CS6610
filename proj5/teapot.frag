#version 330 core 

in vec2 out_coord;
in vec3 world_pos;
in vec3 world_normal;

uniform vec3 in_eye;
uniform vec3 in_light;

uniform sampler2D basecolorTex;
uniform sampler2D specularcolorTex;

out vec4 color;
void main(){
    vec3 ambient = vec3(0.1, 0.1, 0.1);
    vec3 specular = vec3(1, 1, 1) * texture(specularcolorTex, out_coord).xyz;
    vec3 albedo = vec3(1, 1, 1) * texture(basecolorTex, out_coord).xyz;
    vec3 radiance = vec3(1, 1, 1);
    float shininess = 5;

    vec3 eye_dir = normalize(in_eye - world_pos);
    vec3 h = normalize(in_light + eye_dir);
    
    vec3 diffuse = (1-ambient) * radiance * albedo * max(0, dot(world_normal, in_light));
    vec3 glossy = (1-ambient) * radiance * specular * pow(max(0, dot(world_normal, h)), shininess);
    color = vec4(glossy + diffuse + ambient, 1);
}