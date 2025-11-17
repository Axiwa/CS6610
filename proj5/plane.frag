#version 330 core 

uniform sampler2D rendertexture;

in vec2 out_texcoord;
out vec4 color;

void main(){
    color = texture(rendertexture, out_texcoord);
}