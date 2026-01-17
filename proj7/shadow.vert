#version 420 core

layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 texcoord;

layout(std140, binding = 3) uniform LightCamera 
{ 
    mat4 view;
    mat4 projection;
    vec4 eyepos;
} lightcamera;

uniform mat4 model;

void main(){
    vec4 wpos = model * vec4(pos, 1);
    gl_Position = lightcamera.projection * lightcamera.view * wpos;
}