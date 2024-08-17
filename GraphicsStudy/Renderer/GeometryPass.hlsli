Texture2D g_albedo : register(t0);
Texture2D g_normal : register(t1);
SamplerState g_sampler : register(s0);

struct Material
{
    float albedo;
    float diffuse;
    float specular;
    float shininess;
};

cbuffer cbPerObject : register(b0)
{
    matrix Model;
    matrix invTranspose;
    Material material;
    bool useNormalMap;
    float3 dummy;
}

cbuffer cbPass : register(b1)
{
    matrix View;
    matrix Projection;
}

struct VSInput
{
    float3 position : POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD;
    float3 tangent : TANGENT;
};

struct PSInput
{
    float4 worldPoition : POSITION;
    float4 svPosition : SV_POSITION;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float2 uv : TEXCOORD;
};

struct PSOutput
{
    float4 position : SV_Target0;
    float4 normal : SV_Target1;
    float4 albedoColor : SV_Target2; 
    float4 specularColor : SV_Target3;
};
