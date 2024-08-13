Texture2D g_ambient : register(t0);
SamplerState g_sampler : register(s0);

struct Material
{
    float3 ambient;
    float shineiness;
    float3 diffuse;
    float dummy1;
    float3 specular;
    float dummy2;
};

cbuffer cbPerObject : register(b0)
{
    matrix Model;
    Material material;
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
};

struct PSInput
{
    float4 worldPoition : POSITION;
    float4 svPosition : SV_POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD;
};

struct PSOutput
{
    float4 position : SV_Target0;
    float4 normal : SV_Target1;
    float4 diffuseColor : SV_Target2;
    float4 ambientColor : SV_Target3;
};
