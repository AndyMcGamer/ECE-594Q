#version 400

attribute vec2 position;
attribute vec2 uv;

uniform mat4 depth_mvp;

void main(){
    gl_Position = depth_mvp * vec4(position, -1, 1);
}