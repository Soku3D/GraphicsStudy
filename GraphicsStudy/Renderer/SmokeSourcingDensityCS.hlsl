#include "Utility.hlsli"
#include "Smoke.hlsli"

RWTexture3D<float> g_density : register(u0);
RWTexture3D<float4> g_velocity : register(u1);
RWTexture3D<int> gBoundaryCondition : register(u2);

float smootherstep(float x, float edge0 = 0.0f, float edge1 = 1.0f)
{
  // Scale, and clamp x to 0..1 range
    x = clamp((x - edge0) / (edge1 - edge0), 0, 1);

    return x * x * x * (3 * x * (2 * x - 5) + 10.0f);
}

[numthreads(16, 16, 4)]
void main( uint3 DTid : SV_DispatchThreadID )
{
    //g_density[DTid.xyz] = max(0.f, g_density[DTid.xyz] - 0.1f);
    uint w, h, d;
    g_density.GetDimensions(w, h, d);
    
    float3 center = float3(0, h / 2, d / 2);
    float l = length(DTid.xyz - center);
    //if (DTid.x == 10 && l < gConstantBuffer.radius)
    if (DTid.x == 10 && l < gConstantBuffer.radius)
    {
        float scale = max(0.f, 1.f - smoothstep(0, gConstantBuffer.radius, l)) * 1.f;
        g_density[DTid.xyz] = scale;
        g_velocity[DTid.xyz] = float4(scale, 0, 0, 0);
    }
}