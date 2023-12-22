#version 400
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec2 tex_coord;
out vec4 frag_color;


void main() {
	frag_color = vec4(tex_coord, 0.0, 1.0);
}