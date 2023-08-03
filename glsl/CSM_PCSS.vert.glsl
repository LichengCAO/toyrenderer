#version 440 core

uniform mat4 u_viewProj;
uniform mat4 u_model;
uniform mat3 u_modelInvTr;
uniform vec3 u_ltDir;

in vec4 vs_pos;
in vec2 vs_uv;
in vec3 vs_norm;

out vec4 fs_pos;
out vec2 fs_uv;
out vec3 fs_norm;


//cascaded shadowmap
uniform sampler2D u_depth0;
uniform sampler2D u_depth1;
uniform sampler2D u_depth2;
uniform sampler2D u_depth3;
uniform vec3 u_sphere0;
uniform vec3 u_sphere1;
uniform vec3 u_sphere2;
uniform vec3 u_sphere3;
uniform float u_radius0;
uniform float u_radius1;
uniform float u_radius2;
uniform float u_radius3;
uniform mat4 u_ltViewProj0;
uniform mat4 u_ltViewProj1;
uniform mat4 u_ltViewProj2;
uniform mat4 u_ltViewProj3;

out vec4 fs_ltClip0;
out vec4 fs_ltClip1;
out vec4 fs_ltClip2;
out vec4 fs_ltClip3;


void main()
{
    fs_uv = vs_uv;
    fs_pos = u_model*vs_pos;
    fs_norm = normalize(u_modelInvTr*vs_norm);
    
    gl_Position = u_viewProj*fs_pos;

    fs_ltClip0 = u_ltViewProj0 * fs_pos;
    fs_ltClip1 = u_ltViewProj1 * fs_pos;
    fs_ltClip2 = u_ltViewProj2 * fs_pos;
    fs_ltClip3 = u_ltViewProj3 * fs_pos;
}
