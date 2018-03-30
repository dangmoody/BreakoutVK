#version 450

layout( location = 0 ) in vec2 in_pos;
layout( location = 1 ) in vec2 in_uv;
layout( location = 2 ) in vec4 in_color;

layout( push_constant ) uniform PushConstant {
	vec2 position;
	vec2 scale;
} ubo_push_constant;

layout( location = 0 ) out vec2 out_uv;
layout( location = 1 ) out vec4 out_color;

out gl_PerVertex {
	vec4 gl_Position;
};

void main() {
	out_uv = in_uv;
	out_color = in_color;

	gl_Position = vec4( in_pos * ubo_push_constant.scale + ubo_push_constant.position, 0.0, 1.0 );
}