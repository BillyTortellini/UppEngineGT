#version 430 core

in vec3 f_dir;
out vec4 o_color;

uniform float u_time;

#define PI 3.1415926535897932384626433832795
float d2r(float d) {
    return d/180*PI;
}

vec3 groundColor = vec3(0.4, 0.22, 0.05);
vec3 skyColor = vec3(0.10, 0.10, 0.7);

float N21(vec2 v)
{
    return fract(343 * fract(sin(v.x*1432.345 + 114.44)*3453.4) + 
                 -22.3434*fract(sin(v.y*5543.123 - 3453.213)*-5344));
}

void main()
{
    vec3 dir = normalize(f_dir);
    vec2 sp = vec2(atan(dir.x, -dir.z), asin(dir.y));
    //sp = vec2(sin(f_dir.x), atan(f_dir.y, -f_dir.z));
    vec3 col = vec3(0);
    float skyVal = smoothstep(-d2r(30), 1, sp.y);
    float groundVal = smoothstep(0, -d2r(3), sp.y);
    float grassVal = smoothstep(-d2r(5), -d2r(30), sp.y);
    vec2 sunPos = vec2(mod(d2r(u_time*30)+d2r(180), d2r(360))-d2r(180), d2r(45));
    float sunVal = smoothstep(d2r(30), 0, distance(sunPos, sp));
    vec3 sunColor = vec3(1, 0.5, 0.5);
    col = mix(col, skyColor, skyVal);
    col = mix(col, sunColor, sunVal);
    col = mix(col, groundColor, groundVal);
    vec3 grassColor = vec3(N21(sp)) * vec3(0,1,0);
    col = mix(col, grassColor, grassVal);


    o_color = vec4(col, 1);
}
