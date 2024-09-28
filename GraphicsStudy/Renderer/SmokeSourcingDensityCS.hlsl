#include "Utility.hlsli"
#include "Smoke.hlsli"

RWTexture3D<float> g_density : register(u0);
RWTexture3D<float4> g_velocity : register(u1);

SamplerState gWarpSampler : register(s0);

ConstantBuffer<SimulationConstant> gConstantBuffer : register(b0);

[numthreads(16, 16, 4)]
void main( uint3 DTid : SV_DispatchThreadID )
{
    g_density[DTid.xyz] = max(0.f, g_density[DTid.xyz] - 0.1f);
    uint w, h, d;
    g_density.GetDimensions(w, h, d);
    
    float3 sourcingPos = float3(10, (h - 1) / 2, (d - 1) / 2);
    float l = length(DTid.xyz - sourcingPos);
    //if (DTid.x == 10 && l < gConstantBuffer.radius)
    if (DTid.x == 10 && l < gConstantBuffer.radius && gConstantBuffer.time < 2.f)
    {
        float scale = max(0.f, 1.f - smoothstep(0, gConstantBuffer.radius, l));
        g_density[DTid.xyz] += scale;
        g_velocity[DTid.xyz].x += scale;
    }
}