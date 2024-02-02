#version 450 core
layout(location=0)out vec4 fColor;
layout(set=0,binding=0)uniform sampler2D sTexture;
layout(location=0)in struct{
    vec4 Color;
    vec2 TexCoord;
    vec2 UISize;
    vec4 UIRadius;
}In;

const float smoothness=.7;

void main()
{
    float alpha=In.Color.a;
    vec4 radius=In.UIRadius;
    
    if(alpha<=0.){
        discard;// no need to do anything else
    }
    
    vec2 pixelPos=In.TexCoord*In.UISize;
    
    // Get the corner radius
    float radiusTopLeft=In.UIRadius.x;
    float radiusTopRight=In.UIRadius.y;
    float radiusBottomLeft=In.UIRadius.z;
    float radiusBottomRight=In.UIRadius.w;
    
    if(radiusTopLeft>0.&&alpha>0.){
        if(pixelPos.x<radiusTopLeft&&pixelPos.y<radiusTopLeft){
            alpha*=1.-smoothstep(radiusTopLeft-smoothness,radiusTopLeft+smoothness,length(pixelPos-vec2(radiusTopLeft,radiusTopLeft)));
        }
    }
    
    if(radiusTopRight>0.&&alpha>0.){
        float xMax=In.UISize.x-radiusTopRight;
        if(pixelPos.x>xMax&&pixelPos.y<radiusTopRight){
            alpha*=1.-smoothstep(radiusTopRight-smoothness,radiusTopRight+smoothness,length(pixelPos-vec2(xMax,radiusTopRight)));
        }
    }
    
    if(radiusBottomLeft>0.&&alpha>0.){
        float yMax=In.UISize.y-radiusBottomLeft;
        if(pixelPos.x<radiusBottomLeft&&pixelPos.y>yMax){
            alpha*=1.-smoothstep(radiusBottomLeft-smoothness,radiusBottomLeft+smoothness,length(pixelPos-vec2(radiusBottomLeft,yMax)));
        }
    }
    
    if(radiusBottomRight>0.&&alpha>0.){
        float xMax=In.UISize.x-radiusBottomRight;
        float yMax=In.UISize.y-radiusBottomRight;
        if(pixelPos.x>xMax&&pixelPos.y>yMax){
            alpha*=1.-smoothstep(radiusBottomRight-smoothness,radiusBottomRight+smoothness,length(pixelPos-vec2(xMax,yMax)));
        }
    }
    
    if(alpha<=0.){
        discard;// same as above
    }
    
    fColor=In.Color*texture(sTexture,In.TexCoord);
    fColor.a*=alpha;
}