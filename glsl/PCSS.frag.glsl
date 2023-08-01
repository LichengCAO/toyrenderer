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


#define NUM_SAMPLES 20
#define BLOCKER_SEARCH_NUM_SAMPLES NUM_SAMPLES
#define PCF_NUM_SAMPLES NUM_SAMPLES
#define NUM_RINGS 10
#define EPS 1e-3
#define PI 3.141592653589793
#define PI2 6.283185307179586

//helper function
highp float rand_1to1(highp float x ) { 
  // -1 -1
  return fract(sin(x)*10000.0);
}
highp float rand_2to1(vec2 uv ) { 
  // 0 - 1
	const highp float a = 12.9898, b = 78.233, c = 43758.5453;
	highp float dt = dot( uv.xy, vec2( a,b ) ), sn = mod( dt, PI );
	return fract(sin(sn) * c);
}
float getDepth(sampler2D shadowMap,vec2 uv){
    return texture2D(shadowMap,uv).r;
}
float calBias(vec3 norm,vec3 ltDir){
    float bias = max(0.01 * (1.0 - abs(dot(norm, ltDir))), 0.005);
    return bias;
}
//sample
vec2 poissonDisk[NUM_SAMPLES];
void poissonDiskSamples( const in vec2 randomSeed ) {

  float ANGLE_STEP = PI2 * float( NUM_RINGS ) / float( NUM_SAMPLES );
  float INV_NUM_SAMPLES = 1.0 / float( NUM_SAMPLES );

  float angle = rand_2to1( randomSeed ) * PI2;
  float radius = INV_NUM_SAMPLES;
  float radiusStep = radius;

  for( int i = 0; i < NUM_SAMPLES; i ++ ) {
    poissonDisk[i] = vec2( cos( angle ), sin( angle ) ) * pow( radius, 0.75 );
    radius += radiusStep;
    angle += ANGLE_STEP;
  }
}

//PCSS
float findBlocker( sampler2D shadowMap,  vec2 uv, float curDepth ) {
    //uniformDiskSamples(uv);
    poissonDiskSamples(uv);
    float avgDepth = 0.0;
    float cnt = 0.0;
    for(int i = 0;i<NUM_SAMPLES;++i){
        vec2 sampleCoord = uv + poissonDisk[i]*vec2(0.01);
        float blockerDepth = getDepth(shadowMap,sampleCoord);
        if(curDepth > blockerDepth){
        cnt += 1.0;
        avgDepth += blockerDepth;
        }
    }
    if(cnt<EPS)return 1.0;
    avgDepth = avgDepth/cnt;
    return avgDepth;
}
float PCF(sampler2D shadowMap, vec4 coords, float bias) {
    poissonDiskSamples(coords.xy);
    float ans = 0.0;
    float curDepth = coords.z - bias;
    for(int i = 0;i<NUM_SAMPLES;++i){
        vec4 shadowCoord = coords;
        shadowCoord.xy += poissonDisk[i]*vec2(0.01);
        ans += curDepth < getDepth(shadowMap,shadowCoord.xy)? 1.0: 0.0;
    }
    return ans/float(NUM_SAMPLES);
}
float PCSS(sampler2D shadowMap, vec4 coords, float bias){
    // STEP 1: avgblocker depth
    float curDepth = coords.z - bias;
    float avgDepth = findBlocker( shadowMap,  coords.xy, curDepth );
    // STEP 2: penumbra size
    float filterSize =  ((curDepth - avgDepth)/avgDepth) * 0.01;
    // STEP 3: filtering
    float ans = 0.0;
    for(int i = 0;i<NUM_SAMPLES;++i){
        vec4 shadowCoord = coords;
        shadowCoord.xy += poissonDisk[i]*vec2(filterSize);
        ans += curDepth < getDepth(shadowMap,shadowCoord.xy)? 1.0: 0.0;
    }
    return ans/float(NUM_SAMPLES);
}
float calVisibility(vec4 ltClip, vec3 norm, vec3 ltDir){
    //vs output                                            fs input
    //clip space (-width/2,width/2) -> ndc space (-1,1) -> screen space (0,1)
    //to NDC
    vec3 ndc = ltClip.xyz / ltClip.w;
    //to screen space
    vec3 screen = ndc*0.5f + vec3(0.5f);
    float bias = calBias(norm, ltDir);
    vec4 coords = vec4(screen,1.0);
    return max(PCSS(u_depth,coords,bias),0.2);
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
