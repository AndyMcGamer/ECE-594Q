#version 400

attribute vec2 position;
attribute vec2 uv;

varying vec4 vPos;
varying vec2 vUv;
varying vec3 vNormal;
varying vec4 vShadowCoord;

uniform mat4 mvp;
uniform mat4 depth_mvp;

void main(){
    
    vPos = vec4(position, -1.0, 1);
    gl_Position = mvp * vPos;
    vNormal = vec3(0,0,1);
    vShadowCoord = depth_mvp * vec4(position, -1, 1);
    vUv = uv;

}