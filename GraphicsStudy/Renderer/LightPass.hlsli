Texture2D g_worldPosition : register(t0);
Texture2D g_normal : register(t1);

Texture2D g_ambientColor : register(t2);
Texture2D g_diffuseColor : register(t3);
Texture2D g_specularColor : register(t4); // r : color, g : shineness

SamplerState g_sampler : register(s0);

#define LIGHT_NUM 30

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
    float3 specular;
}

float4 ComputeLight(int lightIndex, float2 uv)
{
    float4 lightColor = float4(0, 0, 0, 0);
    float3 position = g_worldPosition.Sample(g_sampler, uv).xyz;
    float3 normal = g_normal.Sample(g_sampler, uv).xyz;
    float4 ambient = g_ambientColor.Sample(g_sampler, uv);
    float3 diffuse = g_diffuseColor.Sample(g_sampler, uv).xyz;
    float4 specular = g_specularColor.Sample(g_sampler, uv);
    
    float3 toLightDir = normalize(light[lightIndex].position - position);
    float diffuseStrength = clamp(dot(toLightDir, normal), 0.f, 1.f);
    
    float4 ambientColor = ambient;
    float3 diffuseColor = diffuseStrength * diffuse;
    float3 specularColor = float3(1, 1, 1);
    
    float3 toEyeDir = normalize(eyePosition - position);
    
    if (diffuseStrength > 0.f)
    {
        specularColor *= specular.r;
        float3 reflectionDir = diffuseStrength * normal - toLightDir;
        float specularStrength = pow(clamp(dot(reflectionDir, toEyeDir), 0.f, 1.f);
        specularColor *= specularStrength;
        specularColor = pow(specularColor, specular.g);
    }
    else
    {
        specularColor *= 0.f;
    }
   
    return lightColor;
}