#version 440
uniform mat4 u_viewProj;
uniform mat4 u_model;
uniform mat3 u_modelInvTr;

in vec4 vs_pos;
in vec3 vs_norm;

out vec3 fs_norm;

void main(){   
    fs_norm = u_modelInvTr * vs_norm;
    gl_Position = u_viewProj * u_model * vs_pos;
}