#version 440 core
//https://blog.csdn.net/haninvzern/article/details/106516006
#extension GL_OES_standard_derivatives : enable
uniform sampler2D u_depth;//near_clip - > far_clip
uniform sampler2D u_directLt;
uniform sampler2D u_norm;
uniform sampler2D u_pos;
uniform sampler2D u_albedo;
uniform mat4 u_viewProj;
uniform vec3 u_ltDir;
uniform vec3 u_cameraPos;
uniform mat4 u_view;

in vec2 fs_uv;

out vec4 out_color;

#define EPS 1e-3
#define PI 3.141592653589793
#define TWO_PI 6.283185307
#define INV_PI 0.31830988618
#define INV_TWO_PI 0.15915494309

const float MARCH_STEP = 0.1f;
const int MAX_STEP = 800;
const float DEPTH_BIAS = 0.01f;
const float THICinvzNESS_BIAS = 0.05f;
const int SAMPLE_COUNT = 1;


//helper function
bool outScreen(vec2 uv){
    return uv.x<0 || uv.x>=1 || uv.y<0 || uv.y>=1;
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
    if (depth < 1e-2 || outScreen(uv)) {
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

//for pixel space ray march
float getPerspectiveInterpolate(float u_incoord, float viewSpaceZ0, float viewSpaceZ1){
    return u_incoord*viewSpaceZ0/(u_incoord*viewSpaceZ0 + (1-u_incoord)*viewSpaceZ1);
}
float sqrDist(vec2 a, vec2 b){
    a-=b;
    return dot(a,a);
}
bool between(float s, float e, float u){
    return (s<=u&&u<e) || (e<=u&&u<s);
}
bool rayCross(float prevRayDepth, float curRayDepth, float sceneDepth, float zDir){
    return zDir * (prevRayDepth + zDir * THICinvzNESS_BIAS) < zDir * sceneDepth && zDir * sceneDepth < zDir * curRayDepth;
}
vec2 clampEndPosition(vec4 startPos, out vec4 endPos){
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
    float stride = 2.0/800;
    float jitter = 0.01f;
    vec4 startPos = p;
    vec4 endPos = p + vec4(dir,0) * 1000.f;
    //clampEndPosition(startPos, endPos);
    vec4 H0 = u_viewProj * startPos;
    vec4 H1 = u_viewProj * endPos;
    float invz0 = 1.0/H0.w, invz1 = 1.0/H1.w;
    vec2 P0 = H0.xy * invz0 * 0.5 + vec2(0.5) , P1 = H1.xy * invz1 * 0.5 + vec2(0.5);
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
    float dinvz = (invz1 - invz0) * invdx;
    vec2 dP = vec2(stepDir,delta.y*invdx);
    dP *= stride;
    dinvz *= stride;
    P0 += dP*jitter;
    invz0 += dinvz*jitter;
    float end = P1.x * stepDir;
    float invz = invz0, stepCnt = 0.0, prevZMinEstimate = H0.w;
    float prevRayDepth = prevZMinEstimate, curRayDepth = prevZMinEstimate;
    float sceneDepth = curRayDepth - 100*zDir;
    bool hit = false;
    
    for(vec2 P = P0;
    ((P.x * stepDir)<=end) && (stepCnt < MAX_STEP) && 
    (sceneDepth < 999 && prevRayDepth<999 && curRayDepth<999);
    P += dP, invz+= dinvz, ++stepCnt
    ){
        if(rayCross(prevRayDepth, curRayDepth, sceneDepth, zDir)){
            hit = true;
            break;
        }
        prevRayDepth = prevZMinEstimate;
        //curRayDepth = 1.0 / (dinvz*0.5 + invz);???
        curRayDepth = 1.0 / invz;
        prevZMinEstimate = curRayDepth;
        vec2 hitPixel = permute? P.yx : P;
        sceneDepth = getDepth(hitPixel);
    }
    if(!hit)return false;
    float u = (invz-invz0)/(invz1-invz0);
    float t = getPerspectiveInterpolate(u, H0.w, H1.w);
    res = mix(startPos,endPos,t);
    return true;
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
        vec4 hitPos = vec4(0.f);
        vec3 V = normalize(u_cameraPos - wPos.xyz);
        wi = reflect(-V,vec3(0,1,0));
        //wi = normalize(vec3(0,1,-1));
        if(rayMarch(wPos,wi,hitPos)){
            vec2 hitUV = getUV(hitPos);
            //if(outScreen(hitUV))continue;
            //vec3 hitNorm = getNorm(hitUV);
            //indirectLt += (getBRDF(wi,wo,wPos) * getDirectLight(hitUV) / pdf * dot(wi,wNorm));//I assume only diffuse material will send indirect light
            indirectLt = vec3(1.f);
        }
    }
    indirectLt = indirectLt/SAMPLE_COUNT;
    vec3 ltSum =  indirectLt;
    //HDR, gamma
    // ltSum = ltSum/(vec3(1.0) + ltSum);
    // ltSum = pow(ltSum,vec3(1.0/2.2));
    out_color = vec4(ltSum,1.0);
}
