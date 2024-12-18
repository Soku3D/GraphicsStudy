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
}

struct VSInput
{
    uint id : SV_InstanceID;
};

struct GSInput
{
    float3 worldPoition : POSITION;
};
struct PSInput
{
    float4 svPosition : SV_POSITION;
};

float4 ApplyMVPMatrix(float4 position)
{
    position = mul(position, Model);
    position = mul(position, View);
    position = mul(position, Projection);
    
    return position;
    
}