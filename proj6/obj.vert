#version 420 core

layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 texcoord;

struct outdata{
    vec3 world_pos;
    vec3 eye_pos;
    vec2 texcoord;
    vec3 world_normal;
};

out outdata o;

layout(std140, binding = 0) uniform CameraBlock 
{ 
    mat4 view;
    mat4 projection;
    vec4 eyepos;
} camera;

uniform mat4 model;

void main(){
    gl_Position = camera.projection * camera.view * model * vec4(pos, 1.0);
    vec4 wpos = model * vec4(pos, 1);
    o.world_pos = wpos.xyz;
    o.world_normal = mat3(transpose(inverse(model))) *  normal;
    o.texcoord = texcoord.xy;
    o.eye_pos = camera.eyepos.xyz;
}