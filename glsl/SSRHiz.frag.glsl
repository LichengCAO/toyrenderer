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
uniform vec3 u_cameraPos;

in vec2 fs_uv;

out vec4 out_color;

#define EPS 1e-3
#define PI 3.141592653589793
#define TWO_PI 6.283185307
#define INV_PI 0.31830988618
#define INV_TWO_PI 0.15915494309

const float MARCH_STEP = 0.1f;
const int MAX_STEP = 200;
const float THICKNESS_BIAS = 1.f;
const int SAMPLE_COUNT = 16;

const int MAX_LEVEL = 3;
const int MIN_LEVEL = 0;
const int LEVEL_COUNT = 4;


//helper function
bool outScreen(vec2 uv){
    return uv.x<0 || uv.x>1 || uv.y<0 || uv.y>1;
}
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
vec3 cosImportanceSample(vec2 rand, out float pdf){
    //https://zhuanlan.zhihu.com/p/360420413
    //pdf(theta)积分等于rand.x -> theta和rand.x的关系
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

//for acceleration
float getMinDepth(vec2 uv, int level){
    float depth = textureLod(u_depth,uv,level).r;
    if (depth < 1e-2) {
        depth = 1000.0;
    }
    return depth;
}

//for pixel space ray march
//transform u in 2D screen space to u in 3D world space
float getPerspectiveInterpolate(float screen_u, float viewSpaceZ0, float viewSpaceZ1){
    return screen_u*viewSpaceZ0/(screen_u*viewSpaceZ0 + (1-screen_u)*viewSpaceZ1);
}
float sqrDist(vec2 a, vec2 b){
    a-=b;
    return dot(a,a);
}
vec2 clampEndPosition(vec4 startPos,in out vec4 endPos){
    vec4 projStart = u_viewProj * startPos;
    vec4 projEnd = u_viewProj * endPos;
    vec2 startUV = (projStart.xy / projStart.w)/2 + vec2(0.5);
    vec2 endUV = (projEnd.xy / projEnd.w)/2 + vec2(0.5);
    endUV += vec2(sqrDist(startUV,endUV)<0.0001?0.01:0.0);
    if(outScreen(endUV)){
        vec2 diff = endUV - startUV;
        float scale = 1.0;
        if(abs(diff.x)>abs(diff.y)){
            scale = diff.x>0?(1-startUV.x)/diff.x:-startUV.x/diff.x;
        }else{
            scale = diff.y>0?(1-startUV.y)/diff.y:-startUV.y/diff.y;
        }
        float t = getPerspectiveInterpolate(scale,projStart.w,projEnd.w);
        //scale down endP
        endPos = mix(startPos,endPos,t);
        endUV = mix(startUV,endUV,scale);
    }
    return endUV;
}
//https://casual-effects.blogspot.com/2014/08/screen-space-ray-tracing.html
bool rayMarch(vec4 p, vec3 dir, out vec4 res){
    float stride = 1./200;
    vec4 startPos = p;
    vec4 endPos = p + vec4(dir,0) * 100.f;
    clampEndPosition(startPos, endPos);
    vec4 H0 = u_viewProj * startPos;
    vec4 H1 = u_viewProj * endPos;
    float inv_z0 = 1.0/H0.w, inv_z1 = 1.0/H1.w;
    vec2 P0 = H0.xy * inv_z0 * 0.5 + vec2(0.5) , P1 = H1.xy * inv_z1 * 0.5 + vec2(0.5);
    P1 += vec2((sqrDist(P0,P1)<0.0001)? 0.01 :0.0);
    vec2 delta = P1 -P0;
    bool permute = false;
    if(abs(delta.x)<abs(delta.y)){
        permute = true;
        delta = delta.yx;
        P0 = P0.yx;
        P1 = P1.yx;
    }
    
    float stepDir = sign(delta.x);
    float zDir = sign(H1.w - H0.w);
    float invdx = stepDir / delta.x;

    int startLevel = MAX_LEVEL;
    int endLevel = MIN_LEVEL;
    float end = P1.x * stepDir;
    float inv_z = inv_z0, stepCnt = 0.0, prevRayRecord = H0.w;
    float curRayDepth = H0.w,sceneDepth = H0.w + 100 * zDir;

    int level = startLevel;
    float dinv_z = (inv_z1 - inv_z0) * invdx * stride;
    vec2 dP = vec2(stepDir,delta.y*invdx) * stride;

    P0 += dP;
    inv_z += dinv_z;
    vec2 P = P0;
    bool rayPassScene = false;
    for(;
    (P.x * stepDir)<=end && stepCnt < MAX_STEP 
    && sceneDepth < 999 && curRayDepth<999
    ;P += dP, inv_z+= dinv_z, ++stepCnt){
        if(level<endLevel)break;
        curRayDepth = 1.0 / inv_z;
        
        vec2 hitPixel = permute? P.yx : P;
        sceneDepth = getMinDepth(hitPixel,level);

        rayPassScene = (curRayDepth - sceneDepth) * zDir > 0;
        if(rayPassScene){
            --level;
            P -= dP;
            inv_z -= dinv_z;
            dP*=0.5f;
            dinv_z*=0.5f;
        }else if(level<MAX_LEVEL){
            ++level;
            dP*=2.f;
            dinv_z*=2.f;
        }
    }
    float u = (inv_z-inv_z0)/(inv_z1-inv_z0);
    float t = getPerspectiveInterpolate(u, H0.w, H1.w);
    res = mix(startPos,endPos,t);
    return  level<endLevel
            && (curRayDepth - sceneDepth) * zDir < THICKNESS_BIAS   //to avoid ray pass wall
            && sqrDist(P,P0)>0.001 //to avoid ray hit at start point
    ;
}

void main()
{
    vec4 wPos = getPos(fs_uv);
    if(wPos == vec4(0.f,0.f,0.f,1.f)){
        out_color = vec4(getDirectLight(fs_uv),1.0f);
        return;
    }
    vec3 wNorm = getNorm(fs_uv);
    float pdf = 1.f;
    mat3 rotMat = tangentToWorld(wNorm);

    vec3 indirectLt = vec3(0.f);
    
    for(int i = 0;i<SAMPLE_COUNT;++i){
        vec3 wo = vec3(0);//wPos to camera
        vec3 wi = rotMat * cosImportanceSample(randVec2(fs_uv+vec2(i)),pdf);//wPos to hitPos
        //vec3 V = normalize(u_cameraPos - wPos.xyz);wi = reflect(-V,vec3(0,1,0));
        vec4 hitPos = vec4(0.f);
        if(rayMarch(wPos,wi,hitPos)){
            vec2 hitUV = getUV(hitPos);
            indirectLt += (getBRDF(wi,wo,wPos) * getDirectLight(hitUV) / pdf * dot(wi,wNorm));//I assume only diffuse material will send indirect light
        }
    }
    indirectLt = indirectLt/SAMPLE_COUNT;
    vec3 ltSum = getDirectLight(fs_uv) + indirectLt;
    //ltSum = indirectLt;
    //HDR, gamma
    ltSum = ltSum/(vec3(1.0) + ltSum);
    ltSum = pow(ltSum,vec3(1.0/2.2));
    out_color = vec4(ltSum,1.0);
}
