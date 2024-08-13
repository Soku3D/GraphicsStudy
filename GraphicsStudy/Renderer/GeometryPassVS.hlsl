#include "GeometryPass.hlsli"

PSInput main(VSInput input)
{
    PSInput output;
    float4 pos = mul(float4(input.position, 1.f), Model);
    output.worldPoition = pos;
    
    pos = mul(pos, View);
    pos = mul(pos, Projection);
    output.svPosition = pos;
    
    output.uv = input.uv;
    output.normal = input.normal;
    
    return output;
}