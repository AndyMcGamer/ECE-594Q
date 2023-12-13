#version 400
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec2 tex_coord;
out float frag_color;


void main() {
	frag_color = gl_FragCoord.z;
}