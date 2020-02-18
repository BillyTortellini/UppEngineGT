#version 430 core

in vec3 f_dir;
out vec4 o_color;

#define PI 3.1415926535897932384626433832795
float d2r(float d) {
    return d/180*PI;
}

void main()
{
    vec3 dir = normalize(f_dir);
    vec2 sp = vec2(atan(dir.x, -dir.z), asin(dir.y));
    //sp = vec2(sin(f_dir.x), atan(f_dir.y, -f_dir.z));
    vec3 col = vec3(0);
    col += smoothstep(-d2r(30), 1, sp.y) * vec3(0.2, 0.2, 1.0);
    col += smoothstep(0, -d2r(3), sp.y) * vec3(0.4, 0.22, 0.05);
    o_color = vec4(col, 1);
}
