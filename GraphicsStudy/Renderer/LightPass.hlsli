Texture2D g_worldPosition : register(t0);
Texture2D g_normal : register(t1);
Texture2D g_albedoColor : register(t2);
Texture2D g_material : register(t3); // rgb : ao, metalic, roughness

Texture2D g_brdf : register(t4);
TextureCube g_irradianceCube : register(t5);
TextureCube g_albedoCube : register(t6);
TextureCube g_specularCube : register(t7);

SamplerState g_wrapLinearSampler : register(s0);
SamplerState g_clampLinearSampler : register(s1);

#define LIGHT_NUM 1
#define PI 3.14159265359
struct Light
{
    float3 position;
    float fallOfStart;
    float3 direction;
    float fallOfEnd;
};

cbuffer cbLight : register(b0)
{
    Light light[LIGHT_NUM];
    float3 eyePosition;
    float lodLevel;
    
    float gui_ao;
    float gui_metallic;
    float gui_roughness;
    float gui_cubeMapExpose;
}

struct PSInput
{
    float4 position : SV_Position;
    float2 uv : TEXCOORD;
};

float3 FresnelSchrick(float3 F0, float VoH)
{
    float Fc = pow(1 - VoH, 5.f);
    return F0 + (1 - F0) * Fc;
}
float3 EnvBRDFIrradiance(float3 irradiance, float3 albedo, float VoH, float metallic, float3 F0)
{
    float3 F = FresnelSchrick(F0, VoH);
    
    // Specluar가 커지면 Irradiance는 작아진다 ( 에너지 보존 법칙 )
    float3 kd = lerp((1.f - F), 0, metallic);
    
    return kd * irradiance * albedo;
}
float3 EnvBRDFSpecular(float3 specluar, float3 albedo, float NoV, float metallic, float roughness, float3 F0)
{
    float2 LUT = g_brdf.Sample(g_clampLinearSampler, float2(NoV, 1 - roughness)).rg;
    
    return specluar * (LUT.r * F0 + LUT.g);
    //return specluar* (LUT.r * F0 + LUT.g);
    //return (LUT.r * F0);
    // LUT.g NoV가 0인 지점에서 roughness가 낮으면 높은값을 갖는다
    //return LUT.g;

}
float3 EnvBRDF(float3 irradiance, float3 specluar, float3 albedo, float ao, float VoH, float NoV, float metallic, float roughness, float3 F0)
{
    float3 irradianceColor = EnvBRDFIrradiance(irradiance, albedo, VoH, metallic, F0);
    //return irradianceColor;
    float3 specularColor = EnvBRDFSpecular(specluar, albedo, NoV, metallic, roughness, F0);
    //return specularColor;
    
    return (irradianceColor + specularColor) * ao;
}
float3 GGX1(float NoV, float k)
{
    return NoV / (NoV * (1 - k) + k);
}
float3 GGX(float roughness, float NoL, float NoV)
{
    float k = pow(roughness + 1, 2.f) / 8.f;
    
    return GGX1(NoV, k) * GGX1(NoL, k);
}
float NDF(float roughness, float NoH)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float NoH2 = NoH * NoH;
    
    return (a2 / (PI * pow(NoH2 * (a2 - 1.f) + 1.f, 2.f)));
}

float3 ComputeLight(int lightIndex, float roughness, float3 position, float3 N, float3 V, float3 F0, float metallic, float3 albedo)
{
    float3 L = normalize(light[lightIndex].position - position);
    float3 H = normalize(L + V);

    float NoL = max(0.0, dot(N, L));
    float NoV = max(0.0, dot(N, V));
    float NoH = max(0.0, dot(N, H));
    float VoH = max(0.0, dot(V, H));
        
    float3 F = FresnelSchrick(F0, VoH);
    float D = NDF(roughness, NoH);
    float3 G = GGX(roughness, NoL, NoV);
    
    //return (F * D * G) / (4.f * NoL * NoV);
    //float3 specularBRDF =  (F * D * G) / max(1e-5, 4.0 * NoL * NoV);
    float3 specularBRDF = (F * D * G) / max(1e-5, 4.0 * NoL * NoV);
    float3 kd = lerp(float3(1, 1, 1) - F, float3(0, 0, 0), metallic);
    float3 diffuseBRDF = kd * albedo;
    return (specularBRDF + diffuseBRDF) * NoL;
}