#include "SimulationParticles.hlsli"
float smoothstep(float x)
{
    return x * x * (3.0f - 2.0f * x);
}
float4 main(PSInput input) : SV_TARGET
{
    float x = length(input.position - input.center) / input.radius;
    float kd = smoothstep(1.f - x) * 100.f * input.radius;
   // return float4(1, 1, 1, 1);
    return float4(input.color * kd * min(input.life, 1.f), 1);
}