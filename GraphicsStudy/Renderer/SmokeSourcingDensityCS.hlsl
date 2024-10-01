#include "Utility.hlsli"
#include "Smoke.hlsli"

RWTexture3D<float> gDensity : register(u0);
RWTexture3D<float4> gVelocity : register(u1);
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
    gDensity.GetDimensions(w, h, d);
    float3 dim = float3(w, h, d);
    gBoundaryCondition[DTid.xyz] = 0;
    
    float3 center = float3(0.02f, 0.5f, 0.5f) * dim;
    float radius = h * 0.2f;
    float l = length(DTid.xyz - center) / radius;
    //if (DTid.x == 10 && l < gConstantBuffer.radius)
    if (l < 1.f)
    {
        gVelocity[DTid.xyz] = float4(32 * gConstantBuffer.sourceStrength, 0, 0, 0) / 64.0 * float(w);
        gDensity[DTid.xyz] = max(smootherstep(1.0 - l), gDensity[DTid.xyz]);
    }
    
    uint x = DTid.x;
    uint y = DTid.y;
    uint z = DTid.z;
    if (x == 0 || y == 0 || z == 0 || x == w - 1 || y == h - 1 || z == d - 1)
    {
        gBoundaryCondition[DTid.xyz] = -1;
    }
    
    center = float3(0.15f, 0.5f, 0.5f) * dim;
    radius = 0.1 * h;
    
    l = length(float3(DTid.xyz) - center) / radius;
    
    if (l < 1.0)
    {
        gDensity[DTid.xyz] = 0.0;
        gVelocity[DTid.xyz] = float4(0, 0, 0, 0); 
        gBoundaryCondition[DTid.xyz] = -2;  
    }
}