#version 400

varying vec4 vPos;
varying vec2 vUv;
varying vec3 vNormal;
varying vec4 vShadowCoord;

uniform vec4 ambient_color;
uniform float ambient_strength;
uniform vec3 light_position;
uniform float light_intensity;

uniform vec3 eye_pos;
uniform float specular_hardness;
uniform vec3 specular_color;

uniform sampler2D base;
uniform sampler2D shadowMap;

uniform bool useShadow;

float near = 0.1f;
float far = 700.0f;

vec3 calcAmbient(){
    return (ambient_strength * ambient_color).rgb;
}

vec3 calcDiffuse(){
    vec3 baseColor = texture2D(base, vUv.xy).rgb;
    vec3 l = (light_position - vPos.xyz);
    float rSquare = dot(l,l);
    l = normalize(l);
    float ndotl = max(0, dot(vNormal, l));
    return baseColor * (light_intensity / rSquare) * ndotl;
}

vec3 calcSpecular(){
    vec3 l = (light_position - vPos.xyz);
    float rSquare = dot(l,l);
    vec3 v = eye_pos - vPos.xyz;
    vec3 h = (v + l) / sqrt(dot(v+l, v+l));

    float ndoth = max(0, dot(vNormal, h));

    return specular_color * (light_intensity / rSquare) * pow(ndoth, specular_hardness);
}

void main(){
    
    float visibility = 1.0;
    vec3 lightCoords = vShadowCoord.xyz / vShadowCoord.w;
    lightCoords = lightCoords * 0.5 + 0.5;
    if ( texture2D( shadowMap, lightCoords.xy ).r < lightCoords.z){
        visibility = 0.5;
    }
    
    if(useShadow)
        gl_FragColor = vec4(calcAmbient() + calcDiffuse() * (visibility) + calcSpecular(), 1.0);
    else
        gl_FragColor = vec4(calcAmbient() + calcDiffuse() + calcSpecular(), 1.0);
}