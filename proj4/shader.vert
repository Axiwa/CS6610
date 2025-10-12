#version 330 core

layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 in_normal;
layout(location = 2) in vec3 in_tex;

out vec3 out_normal;
out vec3 out_eye;
out vec2 out_texcoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;
uniform vec3 in_eyepos;

void main(){
    gl_Position = proj * view * model * vec4(pos, 1.0);
    out_normal = mat3(transpose(inverse(model))) * in_normal;
    out_eye = in_eyepos - pos;
    out_texcoord = vec2(in_tex.x, in_tex.y);
}