Texture2D g_basic: register(t0);
SamplerState g_sampler : register(s0);

struct PSInput
{
    float4 position : SV_POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD;
};
float4 main(PSInput input) : SV_TARGET
{
    float4 output = g_basic.Sample(g_sampler, input.uv);
    //clip(output.a - 0.1f);
    return output;
    
}