#include "Common.hlsli"

float4 main(PSInput input) : SV_TARGET
{ 
    return ComputePhongShading(input.uv, input.worldPoition, input.normal);   
}