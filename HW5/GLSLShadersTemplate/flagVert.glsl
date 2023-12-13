#version 400

#define PI 3.1415926538

attribute vec2 position;
attribute vec2 uv;

varying vec4 vPos;
varying vec2 vUv;
varying vec3 vNormal;

uniform mat4 mvp;
uniform int period;
uniform float t;


void main(){
    
    vPos = period == 0 ? vec4(position, 0, 1) : vec4(position, 1.0/(2.0 * period) * sin(uv.x * (2 * PI * period) + t), 1);
    gl_Position = mvp * vPos;
    float offset = period == 0 ? 0 : 1.0/(2.0 * period) * sin(uv.x * (2 * PI * period) + t);
    
    vec3 tangent = vec3(1,0,0);
    vNormal = vec3(0,0,1);
    
    vec3 bitangent = cross(vNormal, tangent);

    vec3 posPlusTangent = vec3(position, 0) + tangent * 0.01;
    vec3 posPlusBitangent = vec3(position, 0) + bitangent * 0.01;

    posPlusTangent.z += offset;
    posPlusBitangent.z += offset;

    vec3 modifiedTangent = posPlusTangent - vPos.xyz;
    vec3 modifiedBitangent = posPlusBitangent - vPos.xyz;

    vNormal = normalize(cross(modifiedTangent, modifiedBitangent));
    
    vUv = uv;

}