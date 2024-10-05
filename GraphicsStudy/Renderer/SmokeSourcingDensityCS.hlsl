#include "Utility.hlsli"
#include "Smoke.hlsli"

RWTexture3D<float> density: register(u0);
RWTexture3D<float4> velocity : register(u1);

// boundary conditions
// -1: Dirichlet condition
// -2: Neumann condition
//  0: Full cell
RWTexture3D<int> bc : register(u2);

float smootherstep(float x, float edge0 = 0.0f, float edge1 = 1.0f)
{
  // Scale, and clamp x to 0..1 range
    x = clamp((x - edge0) / (edge1 - edge0), 0, 1);

    return x * x * x * (3 * x * (2 * x - 5) + 10.0f);
}

[numthreads(16, 16, 4)]
void main(uint3 dtID : SV_DispatchThreadID)
{
    uint width, height, depth;
    bc.GetDimensions(width, height, depth);
    
    bc[dtID] = 0;
    
    if (dtID.x == 0 || dtID.y == 0 || dtID.z == 0
        || dtID.x == width - 1 || dtID.y == height - 1 || dtID.z == depth - 1)
    {
        bc[dtID] = -1; // Dirichlet boundary condition
        //density[dtID.xyz] = 0.0;
        //velocity[dtID.xyz] = 0.0;
    }

    // Source
    float3 center = float3(0.02, 0.5, 0.5) / gConstantBuffer.dxBase;
    float3 center2 = float3(1.f-0.02, 0.5, 0.5) / gConstantBuffer.dxBase;
    int radius = 0.2 * height;

    float dist = length(float3(dtID.xyz) - center) / radius;
    float dist2 = length(float3(dtID.xyz) - center2) / radius;
    
    if (dist < 1.0)
    {
        velocity[dtID.xyz] = float4(32 * gConstantBuffer.sourceStrength, 0, 0, 0) / 64.0 * float(width); // scale up velocity
        density[dtID.xyz] = max(smootherstep(1.0 - dist), density[dtID.xyz]);
        //bc[dtID.xyz] = -2; // Neumann
    }
    if (dist2 < 1.0)
    {
        velocity[dtID.xyz] = float4(-32 * gConstantBuffer.sourceStrength, 0, 0, 0) / 64.0 * float(width); // scale up velocity
        density[dtID.xyz] = max(smootherstep(1.0 - dist2), density[dtID.xyz]);
        //bc[dtID.xyz] = -2; // Neumann
    }

    // Object
    center = float3(0.15, 0.5, 0.5) / gConstantBuffer.dxBase;
    radius = 0.1 * height;
    center2 = float3(1.f-0.15, 0.5, 0.5) / gConstantBuffer.dxBase;
    
    dist = length(float3(dtID.xyz) - center) / radius;
    dist2 = length(float3(dtID.xyz) - center2) / radius;
    
    if (dist < 1.0)
    {
        velocity[dtID.xyz] = float4(0, 0, 0, 0) / 64.0 * width;
        density[dtID.xyz] = 0.0;
        bc[dtID.xyz] = -2; // Neumann
    }
    if (dist2 < 1.0)
    {
        velocity[dtID.xyz] = float4(0, 0, 0, 0) / 64.0 * width;
        density[dtID.xyz] = 0.0;
        bc[dtID.xyz] = -2; // Neumann
    }
    //velocity[dtID.xyz] += float4(0, 0.5, 0, 0) * density[dtID.xyz] * gConstantBuffer.deltaTime * width;
    //velocity[dtID.xyz] += float4(0, 1.1, 0, 0) * density[dtID.xyz] * dt * width;

}