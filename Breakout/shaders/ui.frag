#version 450

layout( binding = 0 ) uniform sampler2D sampler_font;

layout( location = 0 ) in vec2 in_uv;
layout( location = 1 ) in vec4 in_color;

layout( location = 0 ) out vec4 out_color;

void main() {
	out_color = texture( sampler_font, in_uv ) * in_color;
}