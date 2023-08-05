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
uniform sampler2D u_depth[4];
uniform vec3 u_sphere[4];
uniform float u_radius[4];
uniform mat4 u_ltViewProj[4];
out vec4 fs_ltClip[4];



void main()
{
    fs_uv = vs_uv;
    fs_pos = u_model*vs_pos;
    fs_norm = normalize(u_modelInvTr*vs_norm);
    
    gl_Position = u_viewProj*fs_pos;

    for(int i = 0;i<4;++i){
        fs_ltClip[i] = u_ltViewProj[i] * fs_pos;
    }
}
