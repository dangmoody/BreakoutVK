#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout( location = 0 ) in vec3 in_position;

layout( binding = 0 ) uniform UBO_static {
	mat4 view_projection;
} ubo_static;

layout( binding = 1 ) uniform UBO_dynamic {
	mat4 model;
	vec4 color;
	vec2 scale;
} ubo_dynamic;

out gl_PerVertex {
	vec4 gl_Position;
};

void main() {
	vec3 position_vertex = in_position * vec3( ubo_dynamic.scale, 1.0 );

	mat4 mvp = ubo_static.view_projection * ubo_dynamic.model;
	gl_Position = mvp * vec4( position_vertex, 1.0 );
}