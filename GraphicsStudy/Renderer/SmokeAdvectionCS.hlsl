#include "Utility.hlsli"
#include "Smoke.hlsli"

RWTexture3D<float> g_density : register(u0);
RWTexture3D<float4> g_velocity : register(u1);

Texture3D<float> g_densityTemp : register(t0);
Texture3D<float4> g_velocityTemp : register(t1);

SamplerState gWarpLinearSampler : register(s0);
SamplerState gWarpPointSampler : register(s1);

ConstantBuffer<SimulationConstant> gConstantBuffer : register(b0);

float3 GetUVW(float3 screenPosition, uint width, uint height, uint depth)
{
    float x = screenPosition.x / (width - 1);
    float y = screenPosition.y / (height - 1);
    float z = screenPosition.z / (depth - 1);
    
    return float3(x, y, z);
}

[numthreads(16, 16, 4)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    uint w, h, d;
    g_density.GetDimensions(w, h, d);
    
    float3 velocity = float3(0.4f, 0, 0);
    //float3 backPos = DTid.xyz;
    float3 uvw = GetUVW(DTid.xyz, w, h, d);
    float3 backPos = uvw - velocity * gConstantBuffer.deltaTime;
    
    g_density[DTid.xyz] += g_densityTemp.SampleLevel(gWarpPointSampler, backPos, 0.f);
    //g_density[DTid.xyz] = g_densityTemp[DTid.xyz];

}