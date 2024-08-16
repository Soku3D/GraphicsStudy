TextureCube g_cubeMap : register(t0);
SamplerState g_sampler : register(s0);

struct PSInput
{
    float3 worldPoition : POSITION;
    float4 svPosition : SV_POSITION;
};

float4 main(PSInput input) : SV_Target
{
    return g_cubeMap.SampleLevel(g_sampler, input.worldPoition, 0.f);
    //return float4(1.f, 0.f, 0.f, 1.f);
}