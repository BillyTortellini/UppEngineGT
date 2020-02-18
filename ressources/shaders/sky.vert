#version 430 core

in vec3 a_Pos;
out vec3 f_dir;

uniform mat4 u_view;
uniform mat4 u_projection;
uniform mat4 u_vp;
uniform mat4 u_mvp;

void main()
{
    mat3 view = mat3(vec3(u_view[0]), vec3(u_view[1]), vec3(u_view[2]));
    //f_dir = vec3(u_projection * vec4(a_Pos, 1));
    f_dir = transpose(view) * a_Pos;
    
    gl_Position = u_projection * vec4(a_Pos, 1);
}
