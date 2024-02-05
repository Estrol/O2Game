#version 450 core

layout(location = 0) out vec4 fColor;
layout(set = 0, binding = 0) uniform sampler2D sTexture;
layout(location = 0) in struct {
    vec4 Color;
    vec2 TexCoord;
} In;

void main() {
    fColor = In.Color * texture(sTexture, In.TexCoord);
}