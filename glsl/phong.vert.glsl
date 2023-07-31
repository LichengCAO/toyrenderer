#version 440 core

uniform vec3 u_ltDir;
uniform mat4 u_viewProj;
uniform mat4 u_model;
uniform mat3 u_modelInvTr;
uniform vec3 u_cameraPos;
uniform sampler2D u_texture;

in vec4 vs_pos;
in vec3 vs_norm;
in vec2 vs_uv;

out vec3 fs_norm;
out vec4 fs_pos;
out vec2 fs_uv;

void main()
{
    fs_pos = u_model * vs_pos;
    fs_uv = vs_uv;
    fs_norm = normalize(u_modelInvTr * vs_norm);
    gl_Position = u_viewProj * fs_pos;
}
