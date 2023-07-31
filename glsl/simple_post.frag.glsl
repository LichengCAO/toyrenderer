#version 440 core

uniform sampler2D u_texture;

in vec2 fs_uv;

out vec4 out_color;

void main()
{
    float albedo = texture(u_texture, fs_uv).r;
    out_color = vec4(vec3(albedo),1.0);
}
