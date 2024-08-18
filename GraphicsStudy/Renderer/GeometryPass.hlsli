Texture2D g_albedo : register(t0);
Texture2D g_ao : register(t1);
Texture2D g_displacement : register(t2);
Texture2D g_metalness : register(t3);
Texture2D g_normal : register(t4);
Texture2D g_roughness : register(t5);

SamplerState g_sampler : register(s0);

struct Material
{
    float albedo;
    float ao;
    float metalic;
    float roughness;
};

cbuffer cbPerObject : register(b0)
{
    matrix Model;
    matrix invTranspose;
    Material material;
    
    bool useAoMap;
    bool useHeightMap;
    bool useMetalnessMap;
    bool useNormalMap;
    bool useRoughnessMap;
    //float3 dummy;
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
    float4 material : SV_Target3; // ao, metalic, roughness
};
