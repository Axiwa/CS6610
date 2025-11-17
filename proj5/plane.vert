#version 330 core

layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 in_tex;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

out vec2 out_texcoord;

void main(){
    gl_Position = proj * view * model * vec4(pos, 1);
    out_texcoord = vec2(in_tex.x, in_tex.y);
}