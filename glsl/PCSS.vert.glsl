#version 440 core

uniform mat4 u_viewProj;
uniform mat4 u_model;
uniform mat3 u_modelInvTr;
uniform mat4 u_ltViewProj;
uniform vec3 u_ltDir;
uniform sampler2D u_depth;

in vec4 vs_pos;
//in vec2 vs_uv;
in vec3 vs_norm;

out vec4 fs_pos;
//out vec2 fs_uv;
out vec3 fs_norm;
out vec4 fs_ltClip;


void main()
{
    //fs_uv = vs_uv;
    fs_pos = u_model*vs_pos;
    fs_norm = normalize(u_modelInvTr*vs_norm);
    fs_ltClip = u_ltViewProj * fs_pos;
    gl_Position = u_viewProj*fs_pos;
}
