#version 440 core
//out vec4 out_color;
uniform int u_mipLevel;
uniform int u_prevWidth;
uniform int u_prevHeight;
uniform sampler2D u_texture;

layout(location = 0) out vec4 o_depth;//near_clip - > far_clip

void main()
{
    ivec2 curCoord = ivec2(gl_FragCoord);//https://www.jianshu.com/p/42b26f83e85f
    ivec2 prevCoord = 2 * curCoord;
    int prevLevel = u_mipLevel - 1;
    float minDepth = texelFetch(u_texture,prevCoord, prevLevel).r;
    int radiusW = (u_prevWidth & 1) != 0?3:2;
    int radiusH = (u_prevHeight & 1) != 0?3:2;
    for(int i =0;i<radiusW ;++i){
        for(int j = 0;j<radiusH;++j){
            minDepth = min(minDepth,texelFetch(u_texture,prevCoord + ivec2(i,j),prevLevel).r);
        }
    }
    o_depth = vec4(minDepth);
}
