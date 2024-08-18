#include "LightPass.hlsli"

float4 main(PSInput input) : SV_TARGET
{
    float3 position = g_worldPosition.Sample(g_wrapLinearSampler, input.uv).xyz;
    float3 normal = g_normal.Sample(g_wrapLinearSampler, input.uv).xyz;
    float4 albedo = g_albedoColor.Sample(g_wrapLinearSampler, input.uv);
    float3 material = g_material.Sample(g_wrapLinearSampler, input.uv);
    
    float ao = material.r;
    float metalic = material.g;
    float roughness = material.b;
    
    ao = gui_ao;
    metalic = gui_metalic;
    roughness = gui_roughness;
    
    float4 irradianceColor = g_irradianceCube.Sample(g_wrapLinearSampler, normal);
    float4 specularColor = g_irradianceCube.Sample(g_wrapLinearSampler, normal);
   
    
    float3 toEye = normalize(eyePosition - position);
    float3 toLight = reflect(-toEye, normal);

    float4 color = float4(1, 1, 1, 1);
    return color;
}