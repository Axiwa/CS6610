#version 420 core

uniform samplerCube envTex;

in vec3 world_pos;
in vec3 eye_pos;

out vec4 color;
void main(){
    vec3 dir = normalize(world_pos - eye_pos);
    vec3 env = texture(envTex, dir).xyz;
    color = vec4(env,1);
}