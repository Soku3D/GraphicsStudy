Texture3D gVolumeDensity : register(t0);

SamplerState wrapLinearSampler : register(s0);
SamplerState wrapPointSampler : register(s1);
SamplerState clampLinearSampler : register(s2);

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
    
    bool useTesslation;
}

cbuffer cbPass : register(b1)
{
    matrix View;
    matrix Projection;
    float3 eyePosition;
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


