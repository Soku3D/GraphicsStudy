#include "CFD.hlsli"
#include "Utility.hlsli"

RWTexture2D<float4> velocity : register(u0);

Texture2D vorticity : register(t0);

SamplerState gWarpLinearSampler : register(s0);
SamplerState gWarpPointSampler : register(s1);

ConstantBuffer<SimulationConstant> gConstantBuffer : register(b0);

[numthreads(32, 32, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    uint width, height;
    vorticity.GetDimensions(width, height);
    uint x = DTid.x;
    uint y = DTid.y;
    
    float dx = 1.f;
    float dy = 1.f;
    
    uint2 left = uint2(((x - 1 < 0) ? (width - 1) : (x - 1)), y);
    uint2 right = uint2(((x + 1 > width - 1) ? (0) : (x + 1)), y);
    uint2 top = uint2(x, ((y - 1 < 0) ? (height - 1) : (y - 1)));
    uint2 bottom = uint2(x, ((y + 1 > height - 1) ? (0) : (y + 1)));
    
    float vl = vorticity[left].x;
    float vr = vorticity[right].x;
    float vt = vorticity[top].x;
    float vb = vorticity[bottom].x;
    float v = vorticity[DTid.xy].x;
    float3 vorticityDivergence = float3((vr - vl) / dx, (vt - vb) / dy, 0.f);
    float l = length(vorticityDivergence);
    float3 N = normalize(vorticityDivergence);
    float3 w = float3(0.f, 0.f, v);
    //float2 vor = float2(N.y * v, N.x * v);
    float2 vor = float2(vorticityDivergence.y * v, vorticityDivergence.x * v);
   
    velocity[DTid.xy].xy += gConstantBuffer.vorticity * vor * gConstantBuffer.deltaTime;
}