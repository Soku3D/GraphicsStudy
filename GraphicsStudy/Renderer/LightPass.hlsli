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
float3 EnvBRDFIrradiance(float3 irradiance, float3 albedo, float VoH, float metallic)
{
    float F0 = lerp(0.04, albedo, metallic);
    float3 F = FresnelSchrick(F0, VoH);
    
    // Specluar가 커지면 Irradiance는 작아진다 ( 에너지 보존 법칙 )
    float3 kd = lerp((1.f - F), 0, metallic);
    
    return kd * irradiance * albedo;
}
float3 EnvBRDFSpecular(float3 specluar, float3 albedo, float NoV, float metallic, float roughness)
{
    float3 F0 = lerp(0.04, albedo, metallic);
    float2 LUT = g_brdf.Sample(g_clampLinearSampler, float2(NoV, 1 - roughness)).rg;
    
    return specluar * (LUT.r * F0 + LUT.g);
    //return specluar* (LUT.r * F0 + LUT.g);
    //return (LUT.r * F0);
    // LUT.g NoV가 0인 지점에서 roughness가 낮으면 높은값을 갖는다
    //return LUT.g;

}
float3 EnvBRDF(float3 irradiance, float3 specluar, float3 albedo, float ao, float VoH, float NoV, float metallic, float roughness)
{
    float3 irradianceColor = EnvBRDFIrradiance(irradiance, albedo, VoH, metallic);
    //return irradianceColor;
    float3 specularColor = EnvBRDFSpecular(specluar, albedo, NoV, metallic, roughness);
    //return specularColor;
    
    return (irradianceColor + specularColor) *ao;
}

float4 ComputeLight(int lightIndex, float2 uv)
{
    float4 lightColor = float4(0, 0, 0, 0);
    float3 position = g_worldPosition.Sample(g_wrapLinearSampler, uv).xyz;
    float3 normal = g_normal.Sample(g_wrapLinearSampler, uv).xyz;
    float4 ambient = g_albedoColor.Sample(g_wrapLinearSampler, uv);
    float3 material = g_material.Sample(g_wrapLinearSampler, uv).rgb;
    
    float3 toLightDir = normalize(light[lightIndex].position - position);
    float diffuseStrength = clamp(dot(toLightDir, normal), 0.f, 1.f);
    
    float4 ambientColor = ambient;
    //return ambientColor;
    float a = 0.5f;
    float3 d = float3(0.2f, 0.2, 0.2);
    float s = 1.f;
    //float shininess = 40.f;
    
    ambientColor.rgb *= a;
    float3 diffuseColor = diffuseStrength * d;
    float3 specularColor = float3(1, 1, 1);
    
    float3 toEyeDir = normalize(eyePosition - position);
    
    if (diffuseStrength > 0.f)
    {
        float shininess =  40.f;
        float spec = s;
        float3 reflectionDir = normalize(diffuseStrength * normal - toLightDir);
        
        specularColor *= spec;
        float specularStrength = pow(clamp(dot(reflectionDir, toEyeDir), 0.f, 1.f), shininess);
        specularColor *= specularStrength;
    }
    else
    {
        specularColor *= 0.f;
    }
    lightColor += ambientColor;
    lightColor += float4(diffuseColor, 0.f);
    lightColor += float4(specularColor, 0.f);
    
    return lightColor;
}