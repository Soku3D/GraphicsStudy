#include "CFD.hlsli"
#include "Utility.hlsli"

RWTexture2D<float4> density : register(u0);
RWTexture2D<float4> velocity : register(u1);

SamplerState gWarpSampler : register(s0);

ConstantBuffer<SimulationConstant> gConstantBuffer : register(b0);

float smootherstep(float x, float edge0 = 0.0f, float edge1 = 1.0f)
{
    x = clamp((x - edge0) / (edge1 - edge0), 0, 1);
    return x * x * x * (3 * x * (2 * x - 5) + 10.0f);
}

[numthreads(32, 32, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    //density[DTid.xy] = max(0.0, density[DTid.xy] - 0.001);
    
    uint width, height;
    density.GetDimensions(width, height);
    float radius = gConstantBuffer.radius;
   
    
    if (gConstantBuffer.i < width)
    {
        float2 mousePos = float2(gConstantBuffer.i, gConstantBuffer.j);
        
        float dist = length(mousePos - DTid.xy) / radius;
        float scale = smootherstep(1.0 - dist);
        density[DTid.xy] += float4(gConstantBuffer.color * scale, 1.f);
        velocity[DTid.xy] += float4(gConstantBuffer.velocity * scale, 0.f);
        
    }
}