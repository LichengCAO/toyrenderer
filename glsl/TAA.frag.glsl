#version 440 core

uniform sampler2D u_texture;
uniform sampler2D u_pos;
uniform sampler2D u_prevFrame;
uniform mat4 u_prevViewProj;

in vec2 fs_uv;

out vec4 out_color;
const float ALPHA = 0.85f;
const int SAMPLE_RADIUS = 3;
const float COLOR_BOX = 1.f;

vec4 getPosition(vec2 uv){
    return texture2D(u_pos,uv);
}
vec2 getPrevUV(vec4 pos){
    vec4 ndc = u_prevViewProj * pos;//NOT pos*u_prevViewProj!!!
    ndc = ndc/ndc.w;
    return ndc.xy * 0.5f + vec2(.5f);
}
bool outScreen(vec2 uv){
    return  
        uv.x <= 0.f
        ||uv.x>1.f
        ||uv.y<=0.f
        ||uv.y>1.f; 
}
void getClamp(vec2 uv, out vec3 minColor, out vec3 maxColor){
    ivec2 coordSize = textureSize(u_texture,0);
    ivec2 coord = ivec2(uv * coordSize);
    vec3 avg = vec3(0.f);
    float cnt = float(2 * SAMPLE_RADIUS) + 1.0;
    cnt = cnt * cnt;
    for(int i = -SAMPLE_RADIUS; i<=SAMPLE_RADIUS;++i){
        for(int j = -SAMPLE_RADIUS; j<=SAMPLE_RADIUS;++j){
            avg += texelFetch(u_texture,coord,0).rgb;
        }
    }
    avg = avg/cnt;
    vec3 sigma = vec3(0.f);
    for(int i = -SAMPLE_RADIUS; i<=SAMPLE_RADIUS;++i){
        for(int j = -SAMPLE_RADIUS; j<=SAMPLE_RADIUS;++j){
            vec3 diff = texelFetch(u_texture,coord,0).rgb - avg;
            sigma += diff * diff;
        }
    }
    sigma = sigma/cnt;
    minColor = clamp(avg - COLOR_BOX*sigma,vec3(0.f),vec3(1.f));
    maxColor = clamp(avg + COLOR_BOX*sigma,vec3(0.f),vec3(1.f));
}

void main()
{
    vec4 wPos = getPosition(fs_uv);
    vec2 prevUV = getPrevUV(wPos);
    bool firstFrame = u_prevViewProj[2][3]==0;
    bool invalidPos = wPos == vec4(0,0,0,1.f);

    if(firstFrame||outScreen(prevUV)||invalidPos){
        out_color = texture2D(u_texture,fs_uv);
    }else{
        vec3 curColor = texture2D(u_texture,fs_uv).rgb;
        vec3 prevColor = texture2D(u_prevFrame,prevUV).rgb;
        vec3 minColor, maxColor;
        getClamp(fs_uv,minColor,maxColor);
        clamp(prevColor,minColor,maxColor);
        vec3 color = mix(curColor,prevColor,ALPHA);
        out_color = vec4(color,1.f);
    }
    
}
