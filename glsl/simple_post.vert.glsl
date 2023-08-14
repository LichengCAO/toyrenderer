#version 440 core

in vec4 vs_pos;
in vec2 vs_uv;

out vec2 fs_uv;

void main()
{
    fs_uv = vs_uv;
    gl_Position = vs_pos;
}
