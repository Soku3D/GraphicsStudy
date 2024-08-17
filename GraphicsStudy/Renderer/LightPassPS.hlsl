#include "LightPass.hlsli"

float4 main(PSInput input) : SV_TARGET
{
    float3 position = g_worldPosition.Sample(g_wrapLinearSampler, input.uv).xyz;
    float3 normal = g_normal.Sample(g_wrapLinearSampler, input.uv).xyz;
    float4 albedo = g_albedoColor.Sample(g_wrapLinearSampler, input.uv);
    
    //return albedo;
    //return float4(normal, 1.f);
    
    float4 color = g_irradianceCube.Sample(g_wrapLinearSampler, normal);
    
    return color;
    for (int i = 0; i < LIGHT_NUM; i++)
    {
        float x = (input.position.x + 1.f)/ 2.f;
        float y = (input.position.y + 1.f) / 2.f;
        y = -y;
        float2 uv = float2(x, y);
        color += ComputeLight(i, uv);
    }
    return color;
}