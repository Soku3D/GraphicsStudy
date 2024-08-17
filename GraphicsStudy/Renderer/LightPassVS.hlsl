struct PSInput
{
    float4 position : SV_Position;
    float2 uv : TEXCOORD;
};

PSInput main(float3 pos : POSITION)
{
    PSInput output;
    output.position = float4(pos, 1.f);
    float2 uv = output.position.xy + float2(1, -1);
    uv /= 2.f;
    uv.y *= -1.f;
    output.uv = uv;
    return output;
}