cbuffer cbPerObject : register(b0)
{
    matrix Model;
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
    float4 position : SV_POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD;
};
PSInput main(VSInput input)
{
    PSInput output;
    float4 position;

    position = mul(float4(input.position, 1.f), Model);
    position = mul(position, View);
    position = mul(position, Projection);

    output.position = position;
    output.normal = input.normal;
    output.uv = input.uv;
    
    return output;
}