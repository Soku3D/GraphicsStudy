#include "RenderSkeleton.hlsli"

float4 main(PSInput input) : SV_TARGET
{
    float3 red = float3(1, 0, 0);
    float3 white = float3(1, 1, 1);
    float a = input.texcoord.r;
    
    return float4(red * a + white * (1 - a), 1);
}