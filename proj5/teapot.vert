#version 330 core

layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 in_normal;
layout(location = 2) in vec3 in_tex;

out vec2 out_coord;
out vec3 world_pos;
out vec3 world_normal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

void main(){
    gl_Position = proj * view * model * vec4(pos, 1);
    vec4 wpos = model * vec4(pos, 1);
    world_pos = wpos.xyz;
    world_normal = mat3(transpose(inverse(model))) * in_normal;
    out_coord = vec2(in_tex.x, in_tex.y);
}