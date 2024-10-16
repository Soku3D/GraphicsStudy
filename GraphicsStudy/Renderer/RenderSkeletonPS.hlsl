#include "RenderSkeleton.hlsli"

float4 main(PSInput input) : SV_TARGET
{
    float3 red = float3(1, 0, 0);
    float3 color = 0.5f;
    
    float3 L = normalize(float3(-10, 10, 0));
    float lightStrength = max(dot(L, input.normal), 0.f);
    
    int vid1 = input.vId / 4;
    int vid2 = input.vId % 4;
    
    bool clicked = isClicked[vid1][vid2];
    if (clicked)
    {
        color = red;
    }
    color += lightStrength;
    return float4(color, 1.f);
}