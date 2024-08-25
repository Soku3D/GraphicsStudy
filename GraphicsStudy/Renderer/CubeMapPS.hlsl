TextureCube g_cubeMap : register(t0);
SamplerState g_sampler : register(s0);

struct PSInput
{
    float3 worldPoition : POSITION;
    float4 svPosition : SV_POSITION;
};
struct PSOutput
{
    float4 albedo : SV_Target0;
    float4 material : SV_Target1;
};
cbuffer CubeMapConstant : register(b1)
{
    float expose;
    float lodLevel;
};

PSOutput main(PSInput input)
{
    PSOutput output;
    
    float4 cubeColor = g_cubeMap.SampleLevel(g_sampler, input.worldPoition, lodLevel);
    
    output.albedo = float4((expose * cubeColor.rgb), 1.f);
    output.material = float4(0, 0, 0, 1);
    
    return output;
    //return float4(1.f, 0.f, 0.f, 1.f);
}