#include "GeometryPass.hlsli"

PSOutput main(PSInput input)
{
    PSOutput output;
    
    output.position = input.worldPoition;
    output.normal = float4(normalize(input.normal), 1.f);
    output.albedoColor = g_albedo.Sample(g_sampler, input.uv);
    
    if (useNormalMap)
    {
        float3 normal = g_normal.Sample(g_sampler, input.uv).xyz;
        
        float3 T = input.tangent;
        float3 N = input.normal;
        float3 B = cross(N, T);
        float3x3 transformMat = float3x3(T, B, N);
        normal = mul(normal, transformMat);
        
        output.normal = float4(normalize(normal), 1.f);
    }
    output.material = float4(material.albedo, material.metalic, material.roughness, 0.f);
    
    return output;
}