#include "LightPass.hlsli"

//float4 main(PSInput input) : SV_TARGET
//{
    //float4 color = float4(0, 0, 0, 1);
    
    //float3 position = g_worldPosition.Sample(g_wrapLinearSampler, input.uv).xyz;
    //float3 N = g_normal.Sample(g_clampLinearSampler, input.uv).xyz;
    
    //return float4(N, 1);
    //float3 albedo = g_albedoColor.Sample(g_wrapLinearSampler, input.uv).rgb;
    //float3 material = g_material.Sample(g_wrapLinearSampler, input.uv).rgb;
        
    //float ao = material.r;
    //float metallic = material.g;
    //float roughness = material.b;
    
    ////float ao = 1.f;
    ////float metallic = 1.f;
    ////float roughness = 0.3f;
    //float3 F0 = lerp(0.04, albedo, metallic);
    
    //float3 V = normalize(eyePosition - position);
    //float3 L = normalize(reflect(-V, N));
    //float3 H = normalize(V + L);
    //float3 PointLight = float3(0, 0, 0);
    
    //[unroll]
    //for (int i = 0; i < LIGHT_NUM; ++i)
    //{
    //    PointLight += ComputeLight(i, roughness, position, N, V, F0);
    //}
    
    ////color.rgb += EnvLight;
    //color.rgb += PointLight;
    //return color;
    
    //float NoV = dot(N, V);
    //float NoL = dot(N, L);
    //float NoH = dot(N, H);
    //float VoH = dot(V, H);
    

    ////float3 irradianceColor = g_irradianceCube.Sample(g_wrapLinearSampler, N).rgb;
    //float3 irradianceColor = g_specularCube.SampleLevel(g_wrapLinearSampler, N, 10.f).rgb;
    //float lod = pow(roughness, 2.0) * 3.f;
    //float3 specularColor = gui_cubeMapExpose * g_specularCube.SampleLevel(g_wrapLinearSampler, L, lod).rgb;
    
   
    
    //float3 EnvLight = EnvBRDF(irradianceColor, specularColor, albedo, ao, VoH, NoV, metallic, roughness, F0);
    
   
float3 SchlickFresnel(float3 F0, float NdotH)
{
    // TODO: ?????? (5)
    return F0 + (1 - F0) * pow((1 - NdotH), 5.f);
}
float NdfGGX(float NdotH, float roughness)
{
    float alpha = roughness * roughness;
    float alphaSq = alpha * alpha;
    float d = alphaSq / (3.14159f * (pow((pow(NdotH, 2.f) * (alphaSq - 1) + 1.f), 2.f)));
    
    return d;
}
float SchlickG1(float v, float k)
{
    return v / (v * (1 - k) + k);
}
// TODO: ?????? (4)
float SchlickGGX(float NdotI, float NdotO, float roughness)
{
    float k = pow(roughness + 1, 2.f) / 8.f;
    return SchlickG1(NdotI, k) * SchlickG1(NdotO, k);
}

float4 main(PSInput input) : SV_TARGET
{
    float4 color = float4(0, 0, 0, 1);
    
    float3 position = g_worldPosition.Sample(g_wrapLinearSampler, input.uv).xyz;
    float3 normalWorld = g_normal.Sample(g_wrapLinearSampler, input.uv).xyz;
    
    float3 albedo = g_albedoColor.Sample(g_wrapLinearSampler, input.uv).rgb;
    float3 material = g_material.Sample(g_wrapLinearSampler, input.uv).rgb;
    
    //return float4(albedo, 1.f);
    float ao = material.r;
    float metallic = material.g;
    float roughness = material.b;
    
    float3 directLighting = float3(0, 0, 0);
    float3 pixelToEye = normalize(eyePosition - position);

 
    float3 lightPosition = float3(-1, 1, -10);
    metallic = 1.f;
    roughness = 0.13f;
    
    float3 lightVec = lightPosition - position;
    float3 halfway = normalize(pixelToEye + lightVec);
        
    float NdotI = max(0.0, dot(normalWorld, lightVec));
    float NdotH = max(0.0, dot(normalWorld, halfway));
    float NdotO = max(0.0, dot(normalWorld, pixelToEye));
        
    const float3 Fdielectric = 0.04; // ????(Dielectric) ?????? F0
    float3 F0 = lerp(Fdielectric, albedo, metallic);
    float3 F = SchlickFresnel(F0, max(0.0, dot(halfway, pixelToEye)));
    float3 kd = lerp(float3(1, 1, 1) - F, float3(0, 0, 0), metallic);
    float3 diffuseBRDF = kd * albedo;
        
    float3 D = NdfGGX(NdotH, roughness);
    float3 G = SchlickGGX(NdotI, NdotO, roughness);
        
    float3 specularBRDF = (D) / max(1e-5, 4.0 * NdotI * NdotO);

    directLighting += (specularBRDF) * NdotI;
    color = float4(directLighting, 1.0);

    return color;
}

