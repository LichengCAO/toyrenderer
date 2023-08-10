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
const int MAX_STEP = 1;
const float DEPTH_BIAS = 0.2f;
const int SAMPLE_COUNT = 1;

const int MAX_LEVEL = 9;
const int MIN_LEVEL = 0;
const int LEVEL_COUNT = 10;


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
//screenpos: (uvx, uvy, z in camera space)
vec3 getScreenPos(vec4 wPos){
    vec4 clipSpace = u_viewProj * wPos;
    vec4 ndc = clipSpace / clipSpace.w;
    vec2 res = ndc.xy;
    res = res/2.f + vec2(0.5f);
    return vec3(res,clipSpace.w);
}
float getMinDepth(vec2 uv, int level){
    float depth = textureLod(u_depth,uv,level).r;
    if (depth < 1e-2) {
        depth = 1000.0;
    }
    return depth;
}
vec2 getCoordCount(int level){
    return textureSize(u_depth,level);
}
vec2 getCoord(vec2 uv, vec2 coordCount){
    return vec2(floor(uv * coordCount));
}
vec3 intersectDepthPlane(vec3 o, vec3 d, float t){
    return o + d*t;
}
vec3 toNextCoord(vec3 o, vec3 d, vec2 coord, vec2 coordCnt, vec2 crossStep, vec2 crossOffset){
    vec3 intersect = vec3(0.f);
    vec2 index = coord + crossStep;
    vec2 boundary = index/ coordCnt;
    boundary = boundary + crossOffset;

    vec2 delta = boundary - o.xy;
    delta = delta/d.xy;

    float t = min(delta.x,delta.y);
    intersect = intersectDepthPlane(o,d,t);
    return intersect;
}

bool rayMarch(vec4 p, vec3 dir, out vec4 res){
    vec3 srnStart = getScreenPos(p);
    vec3 srnEnd = getScreenPos(p + vec4(dir,0) * 1000.f );
    vec3 srnDir = (srnEnd - srnStart)/length(srnEnd.xy-srnStart.xy);
    vec2 crossStep = vec2(srnDir.x>0?1:-1,srnDir.y>0?1:-1);
    vec2 crossOffset = crossStep/textureSize(u_depth,0)/128;
    crossStep = clamp(crossStep,vec2(0.f),vec2(1.0));
    vec3 raySrn = srnStart;
    float maxTraceX = srnDir.x >=0?(1-srnStart.x)/srnDir.x:-srnStart.x/srnDir.x;
    float maxTraceY = srnDir.y >=0?(1-srnStart.y)/srnDir.y:-srnStart.y/srnDir.y;
    float maxTraceDist = min(maxTraceX,maxTraceY);
    float minZ = srnStart.z;
    float maxZ = srnStart.z + srnDir.z * maxTraceDist;
    float deltaZ = (maxZ - minZ);

    vec3 o = raySrn;
    vec3 d = srnDir * maxTraceDist;

    int startLevel = MAX_LEVEL;
    int endLevel = MIN_LEVEL;
    vec2 startCellCnt = getCoordCount(startLevel);
    vec2 rayCoord = getCoord(raySrn.xy,startCellCnt);

    raySrn = toNextCoord(o,d,rayCoord,startCellCnt,crossStep,crossOffset);
    int level = startLevel;
    int itr = 0;
    bool isBackward = srnDir.z < 0;
    while(level>=endLevel && abs(raySrn.z)<=abs(maxZ) && itr < MAX_STEP)
    {
        vec2 coordCnt = getCoordCount(level);
        vec2 prevCoord = getCoord(raySrn.xy,coordCnt);
        float curMinZ = getMinDepth((prevCoord + vec2(0.5))/coordCnt, level);
        vec3 tmpRay = ((curMinZ > raySrn.z) && !isBackward) ? intersectDepthPlane(o,d,(curMinZ - minZ)/deltaZ):raySrn;
        vec2 newCoord = getCoord(tmpRay.xy,coordCnt);
        float thickness = level == 0? (raySrn.z - curMinZ):0;
        bool crossed = (isBackward && (curMinZ > raySrn.z)) || (prevCoord != newCoord) || (thickness > MAX_THICKNESS);
        raySrn = crossed? toNextCoord(o,d,prevCoord,coordCnt,crossStep,crossOffset):tmpRay;
        level = crossed ? min(MAX_LEVEL, level + 1) : level - 1;
        ++itr;
    }
    float u = (raySrn.z - srnStart.z) / (srnEnd.z - srnStart.z);
    res = mix(p,p+vec4(dir,0)*1000.f,u);
    return level < endLevel;
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
        wi = normalize(vec3(0,1,-1));
        vec4 hitPos = vec4(0.f);
        if(rayMarch(wPos,wi,hitPos)){
            vec2 hitUV = getUV(hitPos);
            indirectLt += (getBRDF(wi,wo,wPos) * getDirectLight(hitUV) / pdf * dot(wi,wNorm));//I assume only diffuse material will send indirect light
            indirectLt = vec3(1.f);
        }
    }
    indirectLt = indirectLt/SAMPLE_COUNT;
    vec3 ltSum = getDirectLight(fs_uv) + indirectLt;
    ltSum = indirectLt;
    //HDR, gamma
    // ltSum = ltSum/(vec3(1.0) + ltSum);
    // ltSum = pow(ltSum,vec3(1.0/2.2));
    out_color = vec4(ltSum,1.0);
}
