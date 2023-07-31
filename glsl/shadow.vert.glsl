#version 440 core

uniform mat4 u_viewProj;
uniform mat4 u_model;

in vec4 vs_pos;


void main()
{
    gl_Position = u_viewProj*u_model*vs_pos;
}
