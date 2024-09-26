#include "Utility.hlsli"

RWTexture3D<float> g_density : register(u0);
RWTexture3D<float4> g_velocity : register(u1);

Texture3D g_densityTemp : register(t0);
Texture3D g_velocityTemp : register(t1);

struct SimulationConstant
{
    float3 color;
    float deltaTime;
    float3 velocity;
    float radius;
    float viscosity;
    float vorticity;
    uint i;
    uint j;
};

SamplerState gWarpLinearSampler : register(s0);
SamplerState gWarpPointSampler : register(s1);

ConstantBuffer<SimulationConstant> gConstantBuffer : register(b0);

[numthreads(16, 16, 4)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    uint w, h, d;
    g_density.GetDimensions(w, h, d);
    
    float3 sourcingPos = float3(1, h - 1 / 2, d - 1 / 2);
    float3 velocity = float3(1, 0, 0);
    float3 backPos = DTid.xyz - velocity * gConstantBuffer.deltaTime;
    
    g_density[DTid.xyz] = g_densityTemp.SampleLevel(gWarpPointSampler, backPos, 0.f);

}