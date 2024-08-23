#include "GeometryPass.hlsli"

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

PSOutput main(PSInput input)
{
    PSOutput output;
    
    output.position = input.worldPoition;
    output.normal = float4(normalize(input.normal), 1.f);
    output.albedoColor = g_albedo.Sample(g_sampler, input.uv);
    float3 albedo = float3(1, 1, 1);
    if (useNormalMap)
    {
        float3 normal = g_normal.Sample(g_sampler, input.uv).xyz;
        
        float3 T = normalize(input.tangent);
        float3 N = normalize(input.normal);
        float3 B = normalize(cross(N, T));
        float3x3 transformMat = float3x3(T, B, N);
        normal = mul(normal, transformMat);
        
        output.normal = float4(normalize(normal), 1.f);
    }
    output.material.r = useAoMap ? g_ao.Sample(g_sampler, input.uv).r : material.ao;
    output.material.g = useMetalnessMap ? g_metalness.Sample(g_sampler, input.uv).r : material.metalic;
    output.material.b = useRoughnessMap ? g_roughness.Sample(g_sampler, input.uv).r : material.roughness;
    output.material.a = 1.f;
    
    float3 normalWorld = output.normal;
    float3 position = output.position;
    float3 lightPosition = float3(-1, 1, -10);
    float3 directLighting = float3(0, 0, 0);
    float3 pixelToEye = normalize(eyePosition - position);
    float metallic = 1.f;
    float roughness = 0.13f;
    
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
    
    
    output.albedoColor = float4(directLighting, 1.f);
    
    return output;
}