cbuffer cb : register(b0)
{
    float offset;
}

float4 main(float4 position :SV_Position) : SV_TARGET
{
    return float4(offset, 1.f, offset, 1.0f);
}