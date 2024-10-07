#include "RenderSkeleton.hlsli"

GSInput main(VSInput input)
{
    GSInput output;
    float3 pos = mul(float4(input.position, 1.f), boneTransforms[6]).xyz;
    output.position = pos;
    
    return output;
}