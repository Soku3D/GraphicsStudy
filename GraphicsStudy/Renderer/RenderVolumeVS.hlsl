#include "Volume.hlsli"

PSInput main(VSInput input)
{
    PSInput output;
    float4 pos = mul(float4(input.position, 1.f), Model);
        
    float3 normal = mul(float4(input.normal, 0.f), invTranspose).xyz;
    
    float3 tangent = mul(float4(input.tangent, 0.f), Model).xyz;
    
    output.worldPoition = pos;
     
    pos = mul(pos, View);
    pos = mul(pos, Projection);
    
    output.svPosition = pos;
    output.uv = input.uv;
    output.tangent = normalize(tangent);
    output.normal = normalize(normal);

    
    return output;
}