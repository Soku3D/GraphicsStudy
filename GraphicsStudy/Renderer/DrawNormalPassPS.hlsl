#include "DrawNormalPass.hlsli"

float4 main(PSInput input) : SV_TARGET
{
    float4 red = float4(1.f, 0.f, 0.f, 1.f);
    float4 green = float4(0.0f, 1.0f, 0.0f, 1.0f);
    float4 blue = float4(0.0f, 0.0f, 1.0f, 1.0f);
   
    return red;
   

}