#version 440 core
uniform mat4 u_viewProj;
uniform mat4 u_model;
uniform mat3 u_modelInvTr;
uniform vec3 u_ltDir;

in vec3 fs_norm;
out vec4 out_color;

void main(){
    vec3 N = normalize(fs_norm);
    out_color = vec4(N*0.5f + vec3(0.5f),1.f);
    //vec3 N = normalize(fs_norm);
    // vec3 wi = -u_ltDir;
    // float lambert = dot(wi,N);
    //out_color = vec4(N*0.5f + vec3(0.5f),1.f)*lambert;
}
