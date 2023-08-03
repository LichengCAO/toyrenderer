#version 440 core
//https://blog.csdn.net/hankern/article/details/106516006
#extension GL_OES_standard_derivatives : enable
uniform mat4 u_viewProj;
uniform mat4 u_model;
uniform mat3 u_viewProjTr;
uniform vec3 u_ltDir;
uniform sampler2D u_texture;
uniform float u_depthBiasA;

//cascaded shadowmap
uniform sampler2D u_depth0;
uniform sampler2D u_depth1;
uniform sampler2D u_depth2;
uniform sampler2D u_depth3;
uniform vec3 u_sphere0;
uniform vec3 u_sphere1;
uniform vec3 u_sphere2;
uniform vec3 u_sphere3;
uniform float u_radius0;
uniform float u_radius1;
uniform float u_radius2;
uniform float u_radius3;

in vec4 fs_ltClip0;
in vec4 fs_ltClip1;
in vec4 fs_ltClip2;
in vec4 fs_ltClip3;


in vec4 fs_pos;
in vec2 fs_uv;
in vec3 fs_norm;

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
//https://zhuanlan.zhihu.com/p/370951892
float calBias(vec3 norm,vec3 ltDir){
    //float level = dFdx(); //if uv changes fast->pos far from screen, we need shadowmap that covers more space
    float biasA = u_depthBiasA;
    float biasB = 1.0 - abs(dot(norm, ltDir));
    float normalBias = 0.045 * biasA * biasB;
    float bias = max(normalBias, 0.005);
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
    float filterSize =  ((curDepth - avgDepth)/avgDepth) * 0.02;
    // STEP 3: filtering
    float ans = 0.0;
    for(int i = 0;i<NUM_SAMPLES;++i){
        vec4 shadowCoord = coords;
        shadowCoord.xy += poissonDisk[i]*vec2(filterSize);
        ans += curDepth < getDepth(shadowMap,shadowCoord.xy)? 1.0: 0.0;
    }
    return ans/float(NUM_SAMPLES);
}

float calVisibility(vec3 norm, vec3 ltDir){
    sampler2D shadowMaps[4] = {u_depth0,u_depth1,u_depth2,u_depth3};
    vec3 spheres[4] = {u_sphere0,u_sphere1,u_sphere2,u_sphere3};
    float radius[4] = {u_radius0,u_radius1,u_radius2,u_radius3};
    vec4 clips[4] = {fs_ltClip0,fs_ltClip1,fs_ltClip2,fs_ltClip3};
    int first = -1;
    int second = -1;
    for(int i = 0;i<4;++i){
        vec3 toSphere = spheres[i] - fs_pos.xyz;
        float r2 = dot(toSphere,toSphere);
        bool inSphere = (r2 - radius[i] * radius[i]) < 0.f;
        if(inSphere){
            if(first == -1)first = i;
            else if(second == -1){
                second = i;
                break;
            }
        }
    }
    //first case
    if(first==-1)return 0.2;
    
    vec4 ltClip = clips[first];
    vec3 ndc = ltClip.xyz / ltClip.w;//to NDC
    vec3 screen = ndc*0.5f + vec3(0.5f);//to screen space
    float bias = calBias(norm, ltDir);
    vec4 coords = vec4(screen,1.0);
    
    //second case
    float f1 = max(PCSS(shadowMaps[first],coords,bias),0.2);
    if(second==-1)return f1;
    
    //third case
    float u = (spheres[second].x - fs_pos.x)/max(spheres[second].x - spheres[first].x,0.1f);
    u = clamp(u,0.f,1.f);
    
    ltClip = clips[second];
    ndc = ltClip.xyz / ltClip.w;//to NDC
    screen = ndc*0.5f + vec3(0.5f);//to screen space
    coords = vec4(screen,1.0);

    float f2 = max(PCSS(shadowMaps[second],coords,bias),0.2);
    return mix(f2,f1,u);
}

void main()
{
    vec3 wi = -u_ltDir;
    vec3 N = normalize(fs_norm);
    float visibility = calVisibility(N,wi);
    float lambert = dot(wi,N);
    vec3 color = texture2D(u_texture,fs_uv).rgb * lambert * visibility;
    out_color = vec4(color,1.0);
    //out_color = vec4(vec3(gl_FragCoord.z),1.0);
}
