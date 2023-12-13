#version 400
#extension GL_ARB_separate_shader_objects : enable

// inputs
layout(location = 0) in vec2 vp;
layout(location = 1) in vec2 texcoords;

// outputs
layout(location = 0) out vec2 out_texcoords;

uniform mat4 mvp;

void main() {
	gl_Position = mvp * vec4(vp, 0.0, 1.0);
	out_texcoords = texcoords;
}