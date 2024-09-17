#include "CFD.hlsli"
#include "Utility.hlsli"

RWTexture2D<float4> density : register(u0);
RWTexture2D<float4> velocity : register(u1);

SamplerState gWarpSampler : register(s0);

struct SimulationConstant
{
    float3 color;
    float deltaTime;
    float3 velocity;
    float radius;
    uint i;
    uint j;
};

ConstantBuffer<SimulationConstant> gConstantBuffer : register(b0);

[numthreads(32, 32, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    density[DTid.xy].xyz = max(0.f, density[DTid.xy].xyz - gConstantBuffer.deltaTime / 5.f);
    
    uint width, height;
    density.GetDimensions(width, height);
    float radius = gConstantBuffer.radius;
   
    
    if (gConstantBuffer.i < width)
    {
        float2 mousePos = float2(gConstantBuffer.i, gConstantBuffer.j);
        
        float dist = length(mousePos - DTid.xy);
        if (dist < radius)
        {
            float scale = smoothstep(0, radius, radius - dist);
            density[DTid.xy] += float4(gConstantBuffer.color * scale, 1.f);
            velocity[DTid.xy] += float4(gConstantBuffer.velocity * scale, 0.f);
            //density[DTid.xy] += float4(gConstantBuffer.velocity, 0.f);
        }
    }
    else
    {
        density[DTid.xy].w = 1.f;
    }
}