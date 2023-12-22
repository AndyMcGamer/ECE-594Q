#version 400

attribute vec3 position;
attribute vec2 uv;
attribute vec3 normal;

varying vec4 vPos;
varying vec2 vUv;
varying vec4 vNormal;

uniform mat4 mvp;
uniform mat4 mvMat;
uniform float[] y_constants;

void main(){
    vPos = vec4(position, 1.0);
    gl_Position = mvp * vPos;
    vUv = uv;
    vNormal = mvMat * vec4(normal, 1.0);
    
}