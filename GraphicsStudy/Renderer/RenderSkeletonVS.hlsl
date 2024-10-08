#include "RenderSkeleton.hlsli"

GSInput main(VSInput input)
{
    GSInput output;
    float3 pos = mul(float4(input.position, 1.f), baseTransforms[input.vId]).xyz;
    output.position = pos;
    
    int pid = input.vId / 4;
    int pid2 = input.vId % 4;
    
    int parentsIdx = parentsIndex[pid][pid2];
    
    if (parentsIdx == -1)
    {
        output.parentsPosition = pos;
    }
    else
    {
        output.parentsPosition = mul(float4(input.position, 1.f), baseTransforms[parentsIdx]).xyz;

    }
    
    return output;
}