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
        normal = normal* 2.f - 1.f;
        normal = normalize(normal);
        //float3 normal = float3(0, 0, 1);
        float3 N = normalize(input.normal);
        float3 T = normalize(input.tangent - dot(input.tangent, N) * N);
        float3 B = normalize(cross(N, T));
        float3x3 transformMat = float3x3(T, B, N);
        normal = mul(normal, transformMat);
        
        output.normal = float4(normalize(normal), 1.f);
    }
    output.material.r = useAoMap ? g_ao.Sample(g_sampler, input.uv).r : material.ao;
    output.material.g = useMetalnessMap ? g_metalness.Sample(g_sampler, input.uv).r : material.metalic;
    output.material.b = useRoughnessMap ? g_roughness.Sample(g_sampler, input.uv).r : material.roughness;
    output.material.a = 1.f;
    
    return output;
}