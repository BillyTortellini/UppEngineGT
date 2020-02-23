#version 430 core

in vec3 a_pos;
in vec3 a_normal;

out vec3 f_normal;
out vec3 f_pos;

uniform mat4 u_MVP;
uniform mat4 u_MODEL;

void main()
{
    gl_Position = u_MVP * vec4(a_pos, 1);
    f_pos = vec3(u_MODEL * vec4(a_pos, 1)); 
    f_normal = a_normal; // TODO Normal matrix multiplication
}
