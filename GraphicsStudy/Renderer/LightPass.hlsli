Texture2D g_worldPosition : register(t0);
Texture2D g_normal : register(t1);
Texture2D g_albedoColor : register(t2);
Texture2D g_specularColor : register(t3); // rgb : ambient,diffuse,specular, a : shineness

Texture2D g_brdf : register(t4);
TextureCube g_irradianceCube : register(t5);
TextureCube g_albedoCube : register(t6);
TextureCube g_specularCube : register(t7);

SamplerState g_wrapLinearSampler : register(s0);
//SamplerState g_clampLinearSampler : register(s1);

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
    float4 specular = g_specularColor.Sample(g_wrapLinearSampler, uv);
    
    float3 toLightDir = normalize(light[lightIndex].position - position);
    float diffuseStrength = clamp(dot(toLightDir, normal), 0.f, 1.f);
    
    float4 ambientColor = ambient;
    ambientColor.rgb *= specular.r;
    float diffuseColor = diffuseStrength * specular.g;
    float3 specularColor = float3(1, 1, 1);
    
    float3 toEyeDir = normalize(eyePosition - position);
    
    if (diffuseStrength > 0.f)
    {
        float shininess = specular.a;
        float spec = specular.b;
        float3 reflectionDir = normalize(diffuseStrength * normal - toLightDir);
        
        specularColor *= spec;
        float specularStrength = pow(clamp(dot(reflectionDir, toEyeDir), 0.f, 1.f), shininess);
        specularColor *= specularStrength;
    }
    else
    {
        specularColor *= 0.f;
    }
   
    return lightColor;
}