#version 440 core
uniform mat4 u_viewProj;
uniform mat4 u_model;
uniform mat3 u_modelInvTr;

in vec3 fs_norm;
out vec4 out_color;

void main(){
    out_color = vec4(fs_norm*0.5f + vec3(0.5f),1.f);
}
