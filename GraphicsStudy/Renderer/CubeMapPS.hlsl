TextureCube g_cubeMap : register(t0);
SamplerState g_sampler : register(s0);

struct PSInput
{
    float3 worldPoition : POSITION;
    float4 svPosition : SV_POSITION;
};
cbuffer CubeMapConstant : register(b1)
{
    float expose;
    float lodLevel;
};

float4 main(PSInput input) : SV_Target
{
    float4 cubeColor = g_cubeMap.SampleLevel(g_sampler, input.worldPoition, lodLevel);
    
    return float4((expose * cubeColor.rgb), 1.f);
    //return float4(1.f, 0.f, 0.f, 1.f);
}