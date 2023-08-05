#version 440 core

uniform sampler2D u_depth;
uniform sampler2D u_directLt;
uniform sampler2D u_norm;
uniform sampler2D u_pos;

in vec2 vs_uv;
in vec4 vs_pos;

out vec2 fs_uv;

void main()
{
    fs_uv = vs_uv;
    gl_Position = vs_pos;
}
