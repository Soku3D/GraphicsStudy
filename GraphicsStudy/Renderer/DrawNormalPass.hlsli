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

struct GSInput
{
    float3 worldPoition : POSITION;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
};
struct PSInput
{
    float4 svPosition : SV_POSITION;
    float3 worldPoition : POSITION;
    float3 coordinate : POISTION2;
};
