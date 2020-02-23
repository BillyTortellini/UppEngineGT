#version 430 core

out vec4 o_color;
in vec2 f_uv;

uniform sampler2D frame;
uniform sampler2D depthMap;
uniform float u_time;

void main()
{
    vec4 col = texelFetch(frame, ivec2(gl_FragCoord), 0);
    //float depth = texture(depthMap, f_uv).r;
    
    o_color = col;
    //o_color = vec4(vec3(depth), 1);
}
