
#include "LightPass.hlsli"

float4 main(PSInput input) : SV_TARGET
{
    float4 color = float4(0, 0, 0, 1);
    
    float3 position = g_worldPosition.Sample(g_wrapLinearSampler, input.uv).xyz;
    float3 N = g_normal.Sample(g_clampLinearSampler, input.uv).xyz;
    float3 albedo = g_albedoColor.Sample(g_wrapLinearSampler, input.uv).rgb;
    float4 materialTex = g_material.Sample(g_wrapLinearSampler, input.uv).rgba;
    float depth = gDepth.Sample(g_wrapPointSampler, input.uv).r;
    depth *= length(position - eyePosition) / 5.f;
    return float4(depth, depth, depth, 1);
    
    if (materialTex.a>=2.f)
    {
        return float4(albedo, 1.f);
    }
    
    float3 material = materialTex.rgb;
    //return float4(material, 1.f);
    float ao = material.r;
    float metallic = material.g;
    float roughness = material.b;
    
    //float ao = 1.f;
    //float metallic = 1.f;
    //float roughness = 0.3f;
    
    float3 F0 = lerp(0.04, albedo, metallic);
    
    float3 V = normalize(eyePosition - position);
    float3 L = normalize(reflect(-V, N));
    float3 H = normalize(V + L);
    float3 PointLight = float3(0, 0, 0);
        
    float NoV = dot(N, V);
    float NoL = dot(N, L);
    float NoH = dot(N, H);
    float VoH = dot(V, H);
    
    [unroll]
    for (int i = 0; i < LIGHT_NUM; ++i)
    {
        PointLight += ComputeLight(i, roughness, position, N, V, F0, metallic, albedo);
    }
    
    //float3 irradianceColor = g_irradianceCube.Sample(g_wrapLinearSampler, N).rgb;
    float3 irradianceColor = g_specularCube.SampleLevel(g_wrapLinearSampler, N, 10.f).rgb;
    float lod = pow(roughness, 2.0) * 3.f;
    float3 specularColor = gui_cubeMapExpose * g_albedoCube.SampleLevel(g_wrapLinearSampler, L, lod).rgb;
    //float3 specularColor = gui_cubeMapExpose * g_specularCube.SampleLevel(g_wrapLinearSampler, N, lod).rgb;
    //return float4(specularColor, 1.f);
    float3 EnvLight = EnvBRDF(irradianceColor, specularColor, albedo, ao, VoH, NoV, metallic, roughness, F0);
    
    color.rgb += EnvLight;
    color.rgb += PointLight;
    return color;
}
