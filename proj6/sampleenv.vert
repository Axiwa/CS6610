#version 420 core

layout(location = 0) in vec3 pos;

layout(std140, binding = 0) uniform CameraBlock 
{ 
    mat4 view;
    mat4 projection;
    vec3 eyepos;
} camera;

out vec3 world_pos;
out vec3 eye_pos;

void main(){
    vec4 wpos = vec4(pos, 1);
    gl_Position = camera.projection * camera.view * wpos;
    world_pos = pos;
    eye_pos = camera.eyepos;
}