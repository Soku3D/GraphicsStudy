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
    float3 normal = mul(float4(input.normal, 0.f), invTranspose).xyz;
    output.normal = normalize(normal);
    
    float3 tangent = mul(float4(input.tangent, 0.f), Model).xyz;
    output.tangent = normalize(tangent);
    
    return output;
}