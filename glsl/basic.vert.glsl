#version 440
uniform vec3 u_color;
in vec4 vs_pos;

void main()
{
    gl_Position = vs_pos;
}
