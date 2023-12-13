#version 400
#define PI 3.1415926538

attribute vec2 position;
attribute vec2 uv;

uniform mat4 depth_mvp;
uniform int period;
uniform float t;

void main(){
    vec4 vPos = period == 0 ? vec4(position, 0, 1) : vec4(position, 1.0/(2.0 * period) * sin(uv.x * (2 * PI * period) + t), 1);
    gl_Position = depth_mvp * vPos;
}