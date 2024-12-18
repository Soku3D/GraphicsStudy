struct Material
{
    float albedo;
    float ao;
    float metalic;
    float roughness;
};

cbuffer cbPerObject : register(b1)
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

cbuffer cbPass : register(b0)
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

struct GSInput
{
    float4 worldPoition : POSITION;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
};
struct PSInput
{
    float4 svPosition : SV_POSITION;
    float2 texcoord : TEXCOORD;
};
