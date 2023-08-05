#version 440 core
//https://blog.csdn.net/hankern/article/details/106516006
#extension GL_OES_standard_derivatives : enable
uniform sampler2D u_depth;//near_clip - > far_clip
uniform sampler2D u_directLt;
uniform sampler2D u_norm;
uniform sampler2D u_pos;
uniform sampler2D u_albedo;
uniform mat4 u_viewProj;
uniform vec3 u_ltDir;

in vec2 fs_uv;

out vec4 out_color;

#define EPS 1e-3
#define PI 3.141592653589793
#define TWO_PI 6.283185307
#define INV_PI 0.31830988618
#define INV_TWO_PI 0.15915494309

const float MARCH_STEP = 0.1f;
const int MAX_STEP = 100;
const float DEPTH_BIAS = 0.2f;


//helper function

vec2 getUV(vec4 wPos){
    vec4 clipSpace = u_viewProj * wPos;
    vec4 ndc = clipSpace / clipSpace.w;
    vec2 res = ndc.xy;
    res = res/2.f + vec2(0.5f);
    return res;
}
float getDepth(vec2 uv){
    float depth = texture2D(u_depth,uv).r;
    if (depth < 1e-2) {
        depth = 1000.0;
    }
    return depth;
}
float getDepth(vec4 wPos){
    vec4 clipSpace = u_viewProj * wPos;
    return clipSpace.w;
}
vec4 getPos(vec2 uv){
    return vec4(texture2D(u_pos,uv).xyz,1.0);
}
vec3 getNorm(vec2 uv){
    return texture2D(u_norm,uv).xyz;
}
vec3 getDirectLight(vec2 uv){
    return texture2D(u_directLt,uv).xyz;
}
vec3 getBRDF(vec3 wi,vec3 wo, vec4 wPos){
    //diffuse
    vec2 uv = getUV(wPos);
    vec3 albedo = texture2D(u_albedo,uv).xyz;
    return albedo * INV_PI;
}
mat3 tangentToWorld(vec3 worldNorm){
    vec3 up        = abs(worldNorm.z) < 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);
    vec3 tangent   = normalize(cross(up, worldNorm));
    vec3 bitangent = cross(worldNorm, tangent);
    return mat3(tangent,bitangent,worldNorm);
}

//sample
vec2 randVec2(vec2 p){
    //0 - 1
    float x = fract(sin(dot(p, vec2(127.1, 311.7))) * 43758.5453);
    float y = fract(sin(dot(p, vec2(269.5,183.3))) * 43758.5453);
    return vec2(x,y);
}
//https://zhuanlan.zhihu.com/p/360420413
//pdf(theta)积分等于rand.x -> theta和rand.x的关系
vec3 cosImportanceSample(vec2 rand, out float pdf){
    float sinTheta = sqrt(1.0 - rand.x);
    float cosTheta = sqrt(rand.x);
    float sinPhi = sin(TWO_PI * rand.y);
    float cosPhi = cos(TWO_PI * rand.y);
    float x = sinTheta * cosPhi;
    float y = sinTheta * sinPhi;
    float z = cosTheta;
    pdf = z * INV_PI;
    return vec3(x,y,z);
}

bool rayMarch(vec4 p, vec3 dir, out vec4 res){
    res = p;
    vec4 stepVec = vec4(dir*MARCH_STEP,0.f);
    res += stepVec;
    for(int i = 0;i<MAX_STEP;++i){
        vec2 curUV = getUV(res);
        if(curUV.x<0.f||curUV.x>1.f||curUV.y<0.f||curUV.y>1.f)break;
        float curDepth = getDepth(res);
        float sceneDepth = getDepth(curUV);
        if((sceneDepth+EPS)<curDepth && (curDepth - sceneDepth) < DEPTH_BIAS)return true;
        res += stepVec;
    }
    return false;
}

void main()
{
    vec4 wPos = getPos(fs_uv);
    vec3 wNorm = getNorm(fs_uv);
    float pdf = 1.f;
    vec3 dir = tangentToWorld(wNorm) * cosImportanceSample(randVec2(fs_uv),pdf);
    vec3 ltSum = getDirectLight(fs_uv);
    //ltSum = vec3(0.f);
    
    vec3 wo = vec3(0);//wPos to camera
    vec3 wi = dir;
    
    vec4 hitPos = vec4(0.f);
    if(rayMarch(wPos,dir,hitPos)){
        vec2 hitUV = getUV(hitPos);
        vec3 hitNorm = getNorm(hitUV);
        
        vec3 indirectLt = getDirectLight(hitUV);//I assume only diffuse material will send indirect light
        ltSum += getBRDF(wi,wo,wPos)  * indirectLt /* / pdf * dot(dir,wNorm) */;
    }
    //HDR, gamma
    ltSum = ltSum/(vec3(1.0) + ltSum);
    ltSum = pow(ltSum,vec3(1.0/2.2));
    out_color = vec4(ltSum,1.0);
}
