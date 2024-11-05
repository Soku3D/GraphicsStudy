#include "Cloth.hlsli"

float smoothstep(float x)
{
    return x * x * (3.0f - 2.0f * x);
}
float4 main(PSInput input) : SV_TARGET
{
    float3 l = float3(0, 0, -1);
    float color = dot(normalize(input.normal), l);
    return float4(color, color, color, 1.f);

}