#include "Utility.hlsli"

RWTexture3D<float> g_density : register(u0);
RWTexture3D<float4> g_velocity : register(u1);

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

SamplerState gWarpSampler : register(s0);

ConstantBuffer<SimulationConstant> gConstantBuffer : register(b0);

[numthreads(16, 16, 4)]
void main( uint3 DTid : SV_DispatchThreadID )
{
    uint w, h, d;
    g_density.GetDimensions(w, h, d);
    
    float3 sourcingPos = float3(1, h - 1 / 2, d - 1 / 2);
    float l = length(DTid.xyz - sourcingPos);
    if (DTid.x == 1 && l < gConstantBuffer.radius)
    {
        g_density[DTid.xyz] += (1.f - smoothstep(0, gConstantBuffer.radius, l));
    }

}