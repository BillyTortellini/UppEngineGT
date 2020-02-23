#version 430 core

in vec2 f_uv;
out vec4 o_Color;

uniform vec2 u_mouse;
uniform vec2 u_resolution;
uniform float u_time;
uniform mat4 u_view;
uniform vec3 u_eye;

#define PI 3.14159265359

float d2r(float d) {
    return d/180*PI;
}

float plane(vec3 ro, vec3 rd, vec4 p, out vec3 n) {
    float t = -dot(vec4(ro, 1), p)/dot(vec4(rd, 0), p); 
    n = vec3(p);
    return min(t, 1000);
}

float sphere(vec3 ro, vec3 rd, vec3 c, float r, out vec3 n) {
    float t = dot(c - ro, rd);
    float y = length(c - (ro+t*rd));
    if (y > r) {
        n = vec3(0);
        return -1;
    }
    float x = sqrt(r*r-y*y);
    n = normalize(vec3(ro+rd*(t-x))-c);
    return t - x;
}

vec3 shade(vec3 p, vec3 n, vec3 e, vec3 mat)
{
    vec3 lightPos = vec3(sin(u_time*2.), 0, cos(u_time*2.))*3 + vec3(0, 4, 0);
    vec3 lightCol = vec3(1);
    vec3 lookDir = normalize(p - e);
    vec3 lightDir = normalize(p - lightPos);
    float d = distance(lightPos, p);
    float attenuation = 1/(d*d*.05 + d*0.2);

    vec3 col = vec3(0);
    // Diffuse
    col += max(0, dot(n, -lightDir)) * mat * lightCol;
    // Specular
    col += pow(max(0, dot(reflect(lightDir, n), -lookDir)), 15) * lightCol * .3;
    col *= attenuation;

    return col;
}

void main()
{
    vec2 uv = f_uv;
    uv.x *= u_resolution.x / u_resolution.y;

    vec2 m = u_mouse*2. - 1.;
    m.x *= u_resolution.x / u_resolution.y;
    m.y *= -1;
    float val = smoothstep(0.5, 0.0,distance(uv, m));

    // Raytracing stuff
    vec3 ro = u_eye;
    float fovX = d2r(90);
    float fovY = fovX * u_resolution.y/u_resolution.x;
    vec3 rd = transpose(mat3(u_view)) * normalize(vec3(sin(fovX/2.)*uv.x, sin(fovY/2.)*uv.y, -1));

    vec3 normal;
    float t = plane(ro, rd, vec4(0, 1, 0, 0), normal);
    vec3 mat = vec3(0);
    if (t > 0) {
        mat = vec3(1);
    }

    vec3 spNormal;
    float ts = sphere(ro, rd, vec3(0, 2, 0), 1, spNormal);
    if (ts > 0) {
        normal = spNormal;
        mat = vec3(1, 0, 0);
        t = ts;
    }

    // Shading
    vec3 col = vec3(0);
    col = shade(ro+t*rd, normal, ro, mat); 
    col = pow(col, vec3(1.0/2.2));

    o_Color=vec4(col,1);
}
