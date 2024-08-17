#include "LightPass.hlsli"

float4 main(PSInput input) : SV_TARGET
{
    float3 position = g_worldPosition.Sample(g_wrapLinearSampler, input.uv).xyz;
    float3 normal = g_normal.Sample(g_wrapLinearSampler, input.uv).xyz;
    float4 albedo = g_albedoColor.Sample(g_wrapLinearSampler, input.uv);
    
    //return albedo;
    //return float4(normal, 1.f);
    
    float4 specularColor = g_irradianceCube.Sample(g_wrapLinearSampler, normal);
    float4 color = float4(0.f, 0.f, 0.f, 1.f);
   
    for (int i = 0; i < LIGHT_NUM; i++)
    {
        color += ComputeLight(i, input.uv);
    }
    return color;
}