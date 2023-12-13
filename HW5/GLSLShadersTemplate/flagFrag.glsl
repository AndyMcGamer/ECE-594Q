#version 400
#define PI 3.1415926538

varying vec4 vPos;
varying vec2 vUv;
varying vec3 vNormal;

uniform vec4 ambient_color;
uniform float ambient_strength;
uniform vec3 light_position;
uniform float light_intensity;

uniform int num_stripes;
uniform int grid_dim;

uniform vec3 eye_pos;
uniform float specular_hardness;
uniform vec3 specular_color;

vec4 stripes(int n){
    vec4 red = vec4(1,0,0,1);
    vec4 white = red.xxxx;
    
    if(int(vUv.y*n) % 2 == 0) return red;
    else return white;

}


vec4 stars(int n, vec2 coords){
    vec4 white = vec4(1);
    vec4 blue = vec4(0,0,1,1);

    // if(coords.y < 0.5 && coords.x < 0.5 || coords.y > 0.5 && coords.x > 0.5) return vec4(1,0,0,1);
    // else return white;

    if (int(coords.y * (n-1)) % 2 == int(coords.x * (n+1)) % 2){
        // draw the star
        float x_reductions = floor(coords.x * (n+1));
        float y_reductions = floor(coords.y * (n-1));
        coords = vec2(coords.x - (x_reductions/(n+1)), coords.y - (y_reductions/(n-1)));
        coords = vec2(mix(-1, 1.0, coords.x * (n+1)), mix(-1, 1.0, coords.y * (n-1)));
        float outerRadius = 1.0;
        float innerRadius = 0.5;
        float radius = length(coords);
        float angle = atan(coords.y, coords.x);

        float centralAngle = 2.0 * PI / 5.0;
        float halfAngle = centralAngle / 2.0;

        if(radius > outerRadius) return blue;

        if(angle >= -PI / 2.0 && angle < -PI/2.0 + halfAngle){
            float t = (-PI/2.0 - angle) / -halfAngle;
            vec2 outerCoord = vec2(outerRadius * cos(-PI/2.0), outerRadius * sin(-PI/2.0));
            vec2 innerCoord = vec2(innerRadius * cos(-PI/2.0 + halfAngle), innerRadius * sin(-PI/2.0 + halfAngle));
            vec2 interpolated_pos = mix(innerCoord, outerCoord, (1-t));
            if(radius <= length(interpolated_pos)) return white;
        }

        if(angle >= -PI/2.0 + halfAngle && angle < -PI/2.0 + 2*halfAngle){
            float t = (-PI/2.0 + halfAngle - angle) / -halfAngle;
            vec2 outerCoord = vec2(outerRadius * cos(-PI/2.0 + 2*halfAngle), outerRadius * sin(-PI/2.0 + 2*halfAngle));
            vec2 innerCoord = vec2(innerRadius * cos(-PI/2.0 + halfAngle), innerRadius * sin(-PI/2.0 + halfAngle));
            vec2 interpolated_pos = mix(outerCoord, innerCoord, (1-t));
            if(radius <= length(interpolated_pos)) return white;
        }

        if(angle >= -PI/2.0 + 2*halfAngle && angle < -PI/2.0 + 3*halfAngle){
            float t = (-PI/2.0 + 2*halfAngle - angle) / -halfAngle;
            vec2 outerCoord = vec2(outerRadius * cos(-PI/2.0 + 2*halfAngle), outerRadius * sin(-PI/2.0 + 2*halfAngle));
            vec2 innerCoord = vec2(innerRadius * cos(-PI/2.0 + 3*halfAngle), innerRadius * sin(-PI/2.0 + 3*halfAngle));
            vec2 interpolated_pos = mix(innerCoord, outerCoord, (1-t));
            if(radius <= length(interpolated_pos)) return white;
        }

        if(angle >= -PI/2.0 + 3*halfAngle && angle < -PI/2.0 + 4*halfAngle){
            float t = (-PI/2.0 + 3*halfAngle - angle) / -halfAngle;
            vec2 outerCoord = vec2(outerRadius * cos(-PI/2.0 + 4*halfAngle), outerRadius * sin(-PI/2.0 + 4*halfAngle));
            vec2 innerCoord = vec2(innerRadius * cos(-PI/2.0 + 3*halfAngle), innerRadius * sin(-PI/2.0 + 3*halfAngle));
            vec2 interpolated_pos = mix(outerCoord, innerCoord, (1-t));
            if(radius <= length(interpolated_pos)) return white;
        }

        if(angle >= -PI/2.0 + 4*halfAngle && angle < -PI/2.0 + 5*halfAngle){
            float t = (-PI/2.0 + 4*halfAngle - angle) / -halfAngle;
            vec2 outerCoord = vec2(outerRadius * cos(-PI/2.0 + 4*halfAngle), outerRadius * sin(-PI/2.0 + 4*halfAngle));
            vec2 innerCoord = vec2(innerRadius * cos(-PI/2.0 + 5*halfAngle), innerRadius * sin(-PI/2.0 + 5*halfAngle));
            vec2 interpolated_pos = mix(innerCoord, outerCoord, (1-t));
            if(radius <= length(interpolated_pos)) return white;
        }

        if(angle < -PI/2.0 && angle >= -PI/2.0 - halfAngle){
            float t = (-PI/2.0 - halfAngle - angle)/(-halfAngle);
            vec2 outerCoord = vec2(outerRadius * cos(-PI/2.0), outerRadius * sin(-PI/2.0));
            vec2 innerCoord = vec2(innerRadius * cos(-PI/2.0 - halfAngle), innerRadius * sin(-PI/2.0 - halfAngle));
            vec2 interpolated_pos = mix(outerCoord, innerCoord, (1-t));
            if(radius <= length(interpolated_pos)) return white;
        }

        if(angle < -PI/2.0 - halfAngle && angle >= -PI/2.0 - 2*halfAngle){
            float t = (-PI/2.0 - 2 * halfAngle - angle)/(-halfAngle);
            vec2 outerCoord = vec2(outerRadius * cos(-PI/2.0 - 2*halfAngle), outerRadius * sin(-PI/2.0 - 2*halfAngle));
            vec2 innerCoord = vec2(innerRadius * cos(-PI/2.0 - halfAngle), innerRadius * sin(-PI/2.0 - halfAngle));
            vec2 interpolated_pos = mix(innerCoord, outerCoord, (1-t));
            if(radius <= length(interpolated_pos)) return white;
        }
        
        if(angle < -PI/2.0 - 2*halfAngle && angle >= -PI/2.0 - 3*halfAngle){
            float t = (-PI/2.0 - 3 * halfAngle - angle)/(-halfAngle);
            vec2 outerCoord = vec2(outerRadius * cos(-PI/2.0 - 2*halfAngle), outerRadius * sin(-PI/2.0 - 2*halfAngle));
            vec2 innerCoord = vec2(innerRadius * cos(-PI/2.0 - 3*halfAngle), innerRadius * sin(-PI/2.0 - 3*halfAngle));
            vec2 interpolated_pos = mix(outerCoord, innerCoord, (1-t));
            if(radius <= length(interpolated_pos)) return white;
        }

        if(angle > PI/2.0 && angle <= PI/2.0 + halfAngle){
            float t = (PI/2.0 + halfAngle - angle) / halfAngle;
            vec2 outerCoord = vec2(outerRadius * cos(PI/2.0 + halfAngle), outerRadius * sin(PI/2.0 + halfAngle));
            vec2 innerCoord = vec2(innerRadius * cos(PI/2.0), innerRadius * sin(PI/2.0));
            vec2 interpolated_pos = mix(outerCoord, innerCoord, t);
            if(radius <= length(interpolated_pos)) return white;
        }

        if(angle > PI/2.0 + halfAngle && angle <= PI/2.0 + 2*halfAngle){
            float t = (PI/2.0 + 2*halfAngle - angle) / halfAngle;
            vec2 outerCoord = vec2(outerRadius * cos(PI/2.0 + halfAngle), outerRadius * sin(PI/2.0 + halfAngle));
            vec2 innerCoord = vec2(innerRadius * cos(PI/2.0 + 2*halfAngle), innerRadius * sin(PI/2.0 + 2*halfAngle));
            vec2 interpolated_pos = mix(innerCoord, outerCoord, t);
            if(radius <= length(interpolated_pos)) return white;
        }

        if(angle > PI/2.0 + 2*halfAngle && angle <= PI/2.0 + 3*halfAngle){
            float t = (PI/2.0 + 3*halfAngle - angle) / halfAngle;
            vec2 outerCoord = vec2(outerRadius * cos(PI/2.0 + 3*halfAngle), outerRadius * sin(PI/2.0 + 3*halfAngle));
            vec2 innerCoord = vec2(innerRadius * cos(PI/2.0 + 2*halfAngle), innerRadius * sin(PI/2.0 + 2*halfAngle));
            vec2 interpolated_pos = mix(outerCoord, innerCoord, t);
            if(radius <= length(interpolated_pos)) return white;
        }
        
    }
    return blue;
}

vec4 flagunion(int n){
    float yOffset = 0.027;
    float xOffset = 0.0315;

    if(vUv.x > (1.0-xOffset) || vUv.x < (0.6 + xOffset) || vUv.y > (0.5385 - yOffset) || vUv.y < yOffset) return vec4(0,0,1,1);

    vec2 normalized_coords = vec2(mix(0, 1.0, (vUv.x - (0.6+xOffset))/(0.4-2*xOffset)), mix(0, 1.0, (vUv.y - yOffset) / (0.5385 - yOffset * 2)));
    
    //return vec4(normalized_coords,0,1);
    return stars(n, normalized_coords);
}

vec3 calcAmbient(){
    return (ambient_strength * ambient_color).rgb;
}

vec3 calcDiffuse(){
    vec3 baseColor = ((vUv.x < 0.6 || vUv.y > 0.5385) ? stripes(num_stripes) : flagunion(grid_dim)).rgb;
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
    vec3 result = calcAmbient() + ( calcDiffuse() + calcSpecular() );
    gl_FragColor = vec4(result, 1);
    //gl_FragColor = vec4((ambient + baseColor), 1.0);
    //gl_FragColor = vec4(vUv, 0, 1);
}