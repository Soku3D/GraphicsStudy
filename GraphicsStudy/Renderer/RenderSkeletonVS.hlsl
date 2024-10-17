#include "RenderSkeleton.hlsli"

GSInput main(VSInput input)
{
    GSInput output;
    output.vId = input.vId;
    float4 pos = mul(float4(input.position, 1.f), baseTransforms[input.vId]);
    pos = mul(pos, boneTransforms[input.vId]);
    output.position = pos.xyz;
    
    int pid = input.vId / 4;
    int pid2 = input.vId % 4;
    
    int parentsIdx = parentsIndex[pid][pid2];
    
    if (parentsIdx == -1)
    {
        output.parentsPosition = pos.xyz;
    }
    else
    {
        float4 parentsPos = mul(float4(input.position, 1.f), baseTransforms[parentsIdx]);
        parentsPos = mul(parentsPos, boneTransforms[parentsIdx]);
        output.parentsPosition = parentsPos.xyz;
    }
    
    return output;
}