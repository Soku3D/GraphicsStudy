#include "LightPass.hlsli"

float4 main(PSInput input) : SV_TARGET
{
    float3 position = g_worldPosition.Sample(g_wrapLinearSampler, input.uv).xyz;
    float3 N = g_normal.Sample(g_wrapLinearSampler, input.uv).xyz;
    float3 albedo = g_albedoColor.Sample(g_wrapLinearSampler, input.uv).rgb;
    float3 material = g_material.Sample(g_wrapLinearSampler, input.uv).rgb;
    //return float4(albedo, 1.f);
        
    float ao = material.r;
    float metallic = material.g;
    float roughness = material.b;
    
    //ao = gui_ao;
    //metallic = gui_metallic;
    //roughness = gui_roughness;
    
    float3 V = normalize(eyePosition - position);
    float3 L = normalize(reflect(-V, N));
    float3 H = normalize(V + L);

    float NoV = dot(N, V);
    float NoL = dot(N, L);
    float NoH = dot(N, H);
    float VoH = dot(V, H);
   
    //float3 irradianceColor = g_irradianceCube.Sample(g_wrapLinearSampler, N).rgb;
    float3 irradianceColor = g_specularCube.SampleLevel(g_wrapLinearSampler, N, 10.f).rgb;
    float lod = pow(roughness, 2.0) * 3.f;
    float3 specularColor = gui_cubeMapExpose * g_specularCube.SampleLevel(g_wrapLinearSampler, L, lod).rgb;
    
    float4 color = float4(0, 0, 0, 1);
    color.rgb += EnvBRDF(irradianceColor, specularColor, albedo, ao, VoH, NoV, metallic, roughness);
        
    return color;
}