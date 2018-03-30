#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout( binding = 1 ) uniform UBO_dynamic {
	mat4 model;
	vec4 color;
	vec2 scale;
} ubo_dynamic;

layout( location = 0 ) out vec4 out_color;

void main() {
	out_color = ubo_dynamic.color;
}