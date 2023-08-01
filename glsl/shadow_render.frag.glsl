#version 440 core

uniform mat4 u_viewProj;
uniform mat4 u_model;
uniform mat3 u_viewProjTr;
uniform mat4 u_ltViewProj;
uniform vec3 u_ltDir;
uniform sampler2D u_depth;

in vec4 fs_pos;
//in vec2 fs_uv;
in vec3 fs_norm;
in vec4 fs_ltClip;

out vec4 out_color;

//vs output                                            fs input
//clip space (-width/2,width/2) -> ndc space (-1,1) -> screen space (0,1)

float calBias(vec3 norm,vec3 ltDir){
    float bias = max(0.01 * (1.0 - abs(dot(norm, ltDir))), 0.005);
    return bias;
}

float calVisibility(vec4 ltClip, vec3 norm, vec3 ltDir){
    //to NDC
    vec3 ndc = ltClip.xyz / ltClip.w;
    //to screen space
    vec3 screen = ndc*0.5f + vec3(0.5f);
    float closeDepth = texture(u_depth,screen.xy).r;
    float curDepth = screen.z;
    float bias = calBias(norm, ltDir);
    return closeDepth < (curDepth - 0.005)? 0.2 : 1.0;
}

void main()
{
    vec3 wi = -u_ltDir;
    vec3 N = normalize(fs_norm);
    float visibility = calVisibility(fs_ltClip,N,wi);
    //float visibility = 1.0;
    vec3 color = (N*0.5f + vec3(0.5f)) * visibility;
    out_color = vec4(color,1.0);
    //out_color = vec4(vec3(gl_FragCoord.z),1.0);
}
