Texture2D g_albedo : register(t0);
Texture2D g_ao : register(t1);
Texture2D g_displacement : register(t2);
Texture2D g_metalness : register(t3);
Texture2D g_normal : register(t4);
Texture2D g_roughness : register(t5);

SamplerState g_sampler : register(s0);
SamplerState g_clampSampler : register(s1);

#define NUM_CONTROL_POINTS 4
static const float MAX_TESSLATION = 64.f;
static const float MIN_TESSLATION = 1.f;
static const float FallOfStart = 10.0f;
static const float FallOfEnd = 50.f;


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
    float boundingBoxHalfLengthX;
    float boundingBoxHalfLengthY;

    float boundingBoxHalfLengthZ;
}

cbuffer cbPass : register(b1)
{
    matrix View;
    matrix Projection;
    float3 eyePosition;
}

cbuffer cbPass : register(b2)
{
    float4x4 boneTransforms[60];
    float4x4 baseTransforms[60];
    int parentIndex[60];
}

struct VSInput
{
    float3 position : POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD;
    float3 tangent : TANGENT;
};


struct VSOutput
{
    float3 position : POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD;
    float3 tangent : TANGENT;
};

struct HS_CONSTANT_DATA_OUTPUT
{
    float edges[4] : SV_TessFactor;
    float inside[2] : SV_InsideTessFactor;
};

struct DSInput
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
    float4 material : SV_Target3; // ao, metalic, roughness, isCubeMap
};

float customSaturate(float min, float max, float fallOfStart, float fallOfEnd, float d)
{
    if (d <= fallOfStart)
    {
        return max;
    }
    else if (d >= fallOfEnd)
    {
        return min;
    }
    else
    {
        float dy = min - max;
        float dx = fallOfEnd - fallOfStart;
        
        return (dy / dx) * (d - fallOfEnd) + min;

    }
}