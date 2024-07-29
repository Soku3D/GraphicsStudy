struct PSInput
{
    float4 position : SV_POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD;
};
float4 main(PSInput input) : SV_TARGET
{
    return float4(1.f, 0.f,0.f, 1.0f);
}