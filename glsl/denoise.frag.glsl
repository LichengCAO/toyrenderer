#version 440 core
//https://blog.csdn.net/hankern/article/details/106516006
#extension GL_OES_standard_derivatives : enable
uniform sampler2D u_norm;
uniform sampler2D u_pos;
uniform sampler2D u_texture;

in vec2 fs_uv;

out vec4 out_color;

#define EPS 1e-3
#define PI 3.141592653589793
#define TWO_PI 6.283185307
#define INV_PI 0.31830988618
#define INV_TWO_PI 0.15915494309

const int SAMPLE_RADIUS = 1;
const float SIGMA_COORD = 2048.f;
const float SIGMA_COLOR = 0.72f;
const float SIGMA_NORMAL = 0.02f;
const float SIGMA_PLANE = 0.02;

//https://zhuanlan.zhihu.com/p/552432586
ivec2 getCoord(vec2 uv, ivec2 texSize){
    return ivec2(uv*texSize);
}
vec3 getColor(ivec2 coord){
    return texelFetch(u_texture,coord,0).rgb;
}
vec3 getPos(ivec2 coord){
    return texelFetch(u_pos,coord,0).xyz;
}
vec3 getNorm(ivec2 coord){
    return texelFetch(u_norm,coord,0).xyz;
}

void main()
{
    ivec2 texSize = textureSize(u_pos,0);
    ivec2 coord = getCoord(fs_uv,texSize);//https://zhuanlan.zhihu.com/p/102068376
    vec3 ptColor = getColor(coord);
    vec3 ptPos = getPos(coord);
    vec3 ptNorm = getNorm(coord);
    float weightSum = 0.f;
    vec3 filteredColor = vec3(0.f);
    for(int dx = -SAMPLE_RADIUS; dx<=SAMPLE_RADIUS;++dx){
        for(int dy = -SAMPLE_RADIUS; dy<=SAMPLE_RADIUS;++dy){
            ivec2 curCoord = coord + ivec2(dx,dy);
            vec3 toSample = normalize(getPos(curCoord) - ptPos);
            vec3 sampleColor = getColor(curCoord);
            vec3 colorDiff = ptColor - sampleColor;
            
            float Dnorm = acos(clamp(
                dot(ptNorm,getNorm(curCoord)),
                0.f,1.f
                ));
            float Dcoord = dx*dx + dy*dy;
            float Dcolor = dot(colorDiff,colorDiff);
            float Dplane = max(0,dot(ptNorm,toSample));
            
            float weight = 
            exp(-Dnorm/SIGMA_NORMAL 
                -Dcoord/SIGMA_COORD 
                -Dcolor/SIGMA_COLOR 
                -Dplane/SIGMA_PLANE);
            weightSum += weight;
            filteredColor += sampleColor*weight;
        }
    }
    out_color = vec4(weightSum ==0? vec3(0.f):filteredColor/weightSum,1.0);
}
