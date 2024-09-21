#include "GeometryPass.hlsli"

//PSInput main(VSInput input)
//{
//    PSInput output;
//    float4 pos = mul(float4(input.position, 1.f), Model);
        
//    float3 normal = mul(float4(input.normal, 0.f), invTranspose).xyz;
    
//    float3 tangent = mul(float4(input.tangent, 0.f), Model).xyz;
    
//    if (useHeightMap)
//    {
//        float height = g_displacement.SampleLevel(g_sampler, input.uv, 0).r;
//        height = height * 2.0 - 1.0;
        
//        pos.xyz += float4(0.02f * normal * height, 0.f);
//    }
//    output.worldPoition = pos;
     
//    pos = mul(pos, View);
//    pos = mul(pos, Projection);
    
//    output.svPosition = pos;
//    output.uv = input.uv;
//    output.tangent = normalize(tangent);
//    output.normal = normalize(normal);

    
//    return output;
//}
VSOutput main(VSInput input)
{
    VSOutput output;
    float4 pos = mul(float4(input.position, 1.f), Model);
    output.position = pos.xyz;
    
    float3 normal = mul(float4(input.normal, 0.f), invTranspose).xyz;
    float3 tangent = mul(float4(input.tangent, 0.f), Model).xyz;
    output.tangent = normalize(tangent);
    output.normal = normalize(normal);
    output.uv = input.uv;
    
    return output;
}