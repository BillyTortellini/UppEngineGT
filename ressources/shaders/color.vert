#version 430 core

in vec3 a_Pos;
in vec3 a_Color;
in vec2 a_uv;

uniform mat4 u_MVP;
uniform mat4 u_VP;
uniform float u_time;

out vec3 f_Color;
out vec2 f_uv;

void main()
{
    f_uv = a_uv;
    f_Color = vec3(sin(u_time)/2+.5, cos(u_time*3.0)/2+.5, sin(u_time*2.5)/2+.5);
    f_Color = vec3(0, 0, 1);

    vec3 pos = a_Pos;
    pos += vec3(sin(u_time + pos.y*2)*3, 0, 0);

    gl_Position = u_MVP * vec4(a_Pos, 1.0); 
    //gl_Position = u_VP * vec4(a_Pos*.3, 1.0); 
}
