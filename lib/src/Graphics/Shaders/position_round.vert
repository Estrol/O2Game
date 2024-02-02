#version 450 core

layout(location=0)in vec2 aPosition;
layout(location=1)in vec2 aTexCoord;
layout(location=2)in vec4 aColor;

layout(push_constant)uniform uPushConstant
{
    // Dynamic changes for each texture
    vec4 uUIRadius;
    vec2 uUISize;
    
    // Same for all textures
    vec2 uScale;
    vec2 uTranslate;
}pc;

out gl_PerVertex{vec4 gl_Position;};

layout(location=0)out struct
{
    vec4 Color;
    vec2 TexCoord;
    vec2 UISize;
    vec4 UIRadius;
}Out;

void main()
{
    gl_Position=vec4(aPosition*pc.uScale+pc.uTranslate,0,1);
    Out.Color=aColor;
    Out.TexCoord=aTexCoord;
    Out.UIRadius=pc.uUIRadius;
    Out.UISize=pc.uUISize;
}