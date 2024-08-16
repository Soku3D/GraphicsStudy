Texture2D g_basic : register(t0);
SamplerState g_sampler : register(s0);

struct Light
{
    float3 position;
    float fallOfStart;
    float3 direction;
    float fallOfEnd;
};
struct Material
{
    float3 ambient;
    float shininess;
    float3 diffuse;
    float dummy1;
    float3 specular;
    float dummy2;
};

cbuffer cbPerObject : register(b0)
{
    matrix Model;
}

cbuffer cbPass : register(b1)
{
    matrix View;
    matrix Projection;
}

cbuffer cbLight : register(b2)
{
    Light light[1];
    float3 eyePosition;
    float lod;
}

struct VSInput
{
    float3 position : POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD;
};

struct PSInput
{
    float3 worldPoition : POSITION;
    float4 svPosition : SV_POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD;
};

float4 ComputePhongShading(float2 uv, float3 worldPosition, float3 normal)
{
    float4 ambient = g_basic.SampleLevel(g_sampler, uv, lod);
    
    return ambient;
    
    ambient.rgb *= 0.8f;
    
    float3 toLight = normalize(light[0].position - worldPosition);
    float3 toEye = normalize(eyePosition - worldPosition);
    
    float dotNormalLight = dot(toLight, normal);
    float diffuseStrength = clamp(dotNormalLight, 0.f, 1.f);
    
    float3 diffuseColor = 0.8f * diffuseStrength;
    float3 specularColor = float3(0.f, 0.f, 0.f);
    
    if (dotNormalLight > 0.f)
    {
        float3 reflectionDir = normalize((normal * diffuseStrength * 2.f) - toLight);
        float specularStrength = pow(clamp(dot(toEye, reflectionDir), 0.f, 1.f), 30.f);
        specularColor = specularStrength * 1.f;
    }
    
    ambient.xyz += diffuseColor + specularColor;
    
    return ambient;
}