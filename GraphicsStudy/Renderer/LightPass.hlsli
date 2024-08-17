Texture2D g_worldPosition : register(t0);
Texture2D g_normal : register(t1);
Texture2D g_albedoColor : register(t2);
Texture2D g_material : register(t3); // rgb : albedo, metalic, roughness

Texture2D g_brdf : register(t4);
TextureCube g_irradianceCube : register(t5);
TextureCube g_albedoCube : register(t6);
TextureCube g_specularCube : register(t7);

SamplerState g_wrapLinearSampler : register(s0);
//SamplerState g_clampLinearSampler : register(s1);

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
}

struct PSInput
{
    float4 position : SV_Position;
    float2 uv : TEXCOORD;
};

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