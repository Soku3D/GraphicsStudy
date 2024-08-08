#include "Common.hlsli"

PSInput main(VSInput input)
{
    PSInput output;
    float4 position;

    position = mul(float4(input.position, 1.f), Model);
    output.worldPoition = position.xyz;
    
    position = mul(position, View);
    position = mul(position, Projection);

    output.svPosition = position;
    output.normal = input.normal;
    output.uv = input.uv;
    
    return output;
}