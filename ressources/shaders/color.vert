#version 430 core

in vec3 a_Pos;
in vec3 a_Color;

uniform mat4 u_MVP;
uniform float u_time;

out vec3 f_Color;

void main()
{
    gl_Position = u_MVP * vec4(a_Pos*.3, 1.0); 
    f_Color = a_Color;
    //f_Color = vec3(sin(u_time)/2+.5, cos(u_time*3.0)/2+.5, sin(u_time*2.5)/2+.5);
}
