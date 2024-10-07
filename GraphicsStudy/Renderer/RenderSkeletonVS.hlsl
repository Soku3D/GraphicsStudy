#include "RenderSkeleton.hlsli"

struct VSInput
{
    float3 position : POSITION;
    uint vId : SV_VertexID;
};

float4 main(VSInput input) : POSITION
{
    float4 pos = mul(float4(input.position, 1.f), boneTransforms[input.vId]);
    //pos = mul(pos, boneTransforms[input.vId]);
    
    return pos;
}