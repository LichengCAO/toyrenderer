#version 440 core
//https://blog.csdn.net/hankern/article/details/106516006
#extension GL_OES_standard_derivatives : enable
uniform mat4 u_viewProj;
uniform mat4 u_model;

//cascaded shadowmap
uniform vec3 u_sphere[4];
uniform float u_radius[4];

in vec4 fs_ltClip[4];


in vec4 fs_pos;

out vec4 out_color;


#define EPS 1e-3
//#define BIAS_A 0.25

vec3 c[4] = {vec3(1,0,0),vec3(0,1,0),vec3(0,0,1),vec3(1,1,0)};

float getBlend(vec3 sphere1, float r1, vec3 sphere2, float r2, vec3 pos){
    vec3 s1Tos2 = sphere2 - sphere1;
    vec3 s1ToPos = pos - sphere1;
    float l2 = dot(s1Tos2,s1Tos2);
    float l = sqrt(l2);
    vec3 s1Sin = cross(s1Tos2,s1ToPos);
    float h2 = dot(s1Sin,s1Sin)/l2;

    float s1_2 = r1*r1 - h2;
    float s2_2 = r2*r2 - h2;
    float s1 = sqrt(s1_2);
    float s2 = sqrt(s2_2);

    float s3_2 = dot(s1ToPos,s1ToPos) - h2;
    float s3 = sqrt(s3_2);
    if(dot(s1Tos2,s1ToPos)<0)s3 = -s3;

    float u = 0.5;
    if((s1+s2-l)>0.01) u = (s3+s2-l)/(s1+s2-l);
    u = clamp(u,0,1);
    return u;
}

vec3 calCascadeLevel(){
    int first = -1;
    int second = -1;
    int third = -1;
    for(int i = 0;i<4;++i){
        vec3 toSphere = u_sphere[i] - fs_pos.xyz;
        float l2 = dot(toSphere,toSphere);
        float r2 = u_radius[i] * u_radius[i];
        bool inSphere = r2 > l2;
        if(inSphere){
            if(first == -1){
                first = i;
            }
            else if(second == -1){
                second = i;
                //break;
            }else{
                third = i;
                break;
            }
        }
    }
    
    //first case
    if(first==-1)return vec3(0.f);
    
    vec4 ltClip = fs_ltClip[first];
    vec3 ndc = ltClip.xyz / ltClip.w;//to NDC
    vec3 screen = ndc*0.5f + vec3(0.5f);//to screen space
    vec4 coords = vec4(screen,1.0);
    
    //second case
    vec3 color1 = c[first];
    if(second==-1)return color1;
    
    //third case
    float u12 = getBlend(u_sphere[first],u_radius[first],u_sphere[second],u_radius[second],fs_pos.xyz);
    
    ltClip = fs_ltClip[second];
    ndc = ltClip.xyz / ltClip.w;//to NDC
    screen = ndc*0.5f + vec3(0.5f);//to screen space
    coords = vec4(screen,1.0);

    vec3 color12 = mix(c[first],c[second],u12);
    if(third == -1)return color12;

    float u13 = getBlend(u_sphere[first],u_radius[first],u_sphere[third],u_radius[third],fs_pos.xyz);
    float u23 = getBlend(u_sphere[second],u_radius[second],u_sphere[third],u_radius[third],fs_pos.xyz);
    vec3 color23 = mix(c[second],c[third],u23);
    return mix(color12,color23,u13);
}

void main()
{
    //float lambert = dot(wi,N);
    out_color = vec4(calCascadeLevel(),1.0);
}
