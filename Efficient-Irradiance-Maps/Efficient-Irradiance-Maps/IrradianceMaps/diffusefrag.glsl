#version 400

varying vec4 vPos;
varying vec2 vUv;
varying vec4 vNormal;

uniform mat4 r_matrix;
uniform mat4 g_matrix;
uniform mat4 b_matrix;

uniform mat4 mvMat;

uniform float environmentStrength;

uniform vec3 lightPos;
uniform vec3 eyePos;
uniform float lightIntensity;
uniform float specularHardness;
uniform vec3 specularColor;
uniform vec3 ambientColor;
uniform float ambientStrength;

uniform int useDiffuse;

vec3 calcLight(vec3 n){
    vec3 baseColor = vec3(1);
    
    vec3 l = (lightPos - vPos.xyz);
    float rSquare = dot(l,l);
    vec3 v = eyePos - vPos.xyz;
    vec3 h = (v + l) / sqrt(dot(v+l, v+l));
    l = normalize(l);
    float ndotl = max(0, dot(n, l));
    vec3 diffuse = baseColor * (lightIntensity / rSquare) * ndotl;

    vec3 ambient = (ambientStrength * ambientColor);

    float ndoth = max(0, dot(n, h));

    vec3 specular = specularColor * (lightIntensity / rSquare) * pow(ndoth, specularHardness);

    return specular + ambient + diffuse;
    
}


float calculateIrradiance(mat4 M, vec3 n){
    vec4 normal = vec4(n, 1);
    return dot(normal, M * normal);
}

void main(){
    vec3 n = normalize((inverse(mvMat) * vNormal).xyz);
    vec4 lightColor = vec4(calcLight(n), 1);
    vec4 envColor = vec4(environmentStrength * calculateIrradiance(r_matrix, n), environmentStrength * calculateIrradiance(g_matrix, n), environmentStrength * calculateIrradiance(b_matrix, n), 1);
    gl_FragColor = vec4((lightColor * useDiffuse + envColor).xyz, 1);
}