#version 450

layout (binding = 0) uniform sampler2D fontSampler;

layout (location = 0) in vec2 inUV;
layout (location = 1) in vec4 inColor;

layout (location = 0) out vec4 outColor;

void main() 
{
    vec4 tex_color = texture(fontSampler, inUV);
	outColor = inColor * tex_color;
}