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
    float3 color = pow(expose * cubeColor.rgb, 1 / 2.2f);
    return float4(color, 1.f);
}