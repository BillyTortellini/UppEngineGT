#version 430 core

in vec3 f_normal;
in vec3 f_pos;

out vec4 o_color;

// Shading uniforms
uniform vec3 u_camPos;
uniform vec3 u_lightDir;
uniform vec3 u_albedo;
uniform vec3 u_ambient;

// Globals
float specStrength = 0.3;
float specPow = 5;

void main()
{
    vec3 normal = normalize(f_normal);
    vec3 col = vec3(0);
    vec3 viewDir = normalize(f_pos - u_camPos);
    col += u_albedo * dot(normal, -u_lightDir); // Diffuse part
    col += u_albedo * u_ambient; // Ambient part
    col += pow(max(0.0, dot(reflect(viewDir, normal), -u_lightDir)), specPow) * specStrength;
    
    // Test
    o_color = vec4(normal*.5 + .5, 1);
    o_color = vec4(u_albedo, 1);
    o_color = vec4(col, 1);
    //o_color = vec4(f_pos, 1);
    //o_color = vec4(viewDir, 1);

    //o_color = vec4(1, 0, 0, 1);
}
