cbuffer cbPass : register(b0)
{
    matrix View;
    matrix Projection;
}
struct VSInput
{
    float3 position : POSITION;
};

struct PSInput
{
    float3 worldPoition : POSITION;
    float4 svPosition : SV_POSITION;
};

PSInput main(VSInput input)
{
    PSInput output;
    
    output.worldPoition = input.position;
    
    float3 pos = mul(float4(input.position, 0.f), View).xyz;
    float4 position = mul(float4(pos,1.f), Projection);
    output.svPosition = position;
    
    return output;

}