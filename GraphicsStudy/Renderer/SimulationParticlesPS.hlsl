#include "SimulationParticles.hlsli"

float4 main(PSInput input) : SV_TARGET
{
    return float4(input.color, 1);
}