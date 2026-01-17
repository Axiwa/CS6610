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
out vec4 posinlightspace;

layout(std140, binding = 0) uniform CameraBlock 
{ 
    mat4 view;
    mat4 projection;
    vec4 eyepos;
} camera;

layout(std140, binding = 3) uniform LightCamera 
{ 
    mat4 view;
    mat4 projection;
    vec4 eyepos;
} lightcamera;

uniform mat4 model;

void main(){
    vec4 wpos = model * vec4(pos, 1);
    gl_Position = camera.projection * camera.view * wpos;
    o.world_pos = wpos.xyz;
    o.eye_pos = camera.eyepos.xyz;
    o.texcoord = texcoord.xy;
    o.world_normal = normal * mat3(inverse(model));

    posinlightspace = lightcamera.projection * lightcamera.view * wpos;
    posinlightspace.z -= 1;
}