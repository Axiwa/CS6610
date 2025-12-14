#version 420 core

layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 texcoord;

struct outdata{
    vec3 world_pos;
    vec2 texcoord;
    vec3 world_normal;
};

out outdata o;

layout(std140, binding = 0) uniform CameraBlock 
{ 
    mat4 model;
    mat4 view;
    mat4 projection;
    vec3 eyepos;
} camera;

void main(){
    gl_Position = camera.projection * camera.view * camera.model * vec4(pos, 1.0);
    vec4 wpos = camera.model * vec4(pos, 1);
    o.world_pos = wpos.xyz;
    o.world_normal = mat3(transpose(inverse(camera.model))) *  normal;
    o.texcoord = texcoord.xy;
}