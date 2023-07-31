#version 440 core

uniform vec3 u_ltDir;
uniform mat4 u_viewProj;
uniform mat4 u_model;
uniform mat3 u_modelInvTr;
uniform vec3 u_cameraPos;
uniform sampler2D u_texture;

in vec3 fs_norm;
in vec4 fs_pos;
in vec2 fs_uv;

out vec4 out_color;

void main()
{
    vec3 wi = -u_ltDir;
    vec3 wo = normalize(u_cameraPos- fs_pos.xyz);
    vec3 h = normalize(wi+wo);
    vec3 n = normalize(fs_norm);
    float NdotH = max(dot(h,n),0.0f);
    float NdotL = max(dot(wi,n),0.0f);
    vec3 albedo = texture(u_texture, fs_uv).xyz;
    vec3 diffuse = albedo * NdotL;
    vec3 specular = vec3(pow(NdotH,20.f));
    out_color = vec4(diffuse + specular,1.0);
}
