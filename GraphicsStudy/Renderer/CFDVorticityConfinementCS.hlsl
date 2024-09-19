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
    
    float dx = 2.f / width;
    float dy = 2.f / height;
    
    uint2 left = uint2(((x == 0) ? (width - 1) : (x - 1)), y);
    uint2 right = uint2(((x == width - 1) ? (0) : (x + 1)), y);
    uint2 top = uint2(x, ((y == 0) ? (height - 1) : (y - 1)));
    uint2 bottom = uint2(x, ((y == height - 1) ? (0) : (y + 1)));
    
    float vl = vorticity[left].x;
    float vr = vorticity[right].x;
    float vt = vorticity[top].x;
    float vb = vorticity[bottom].x;
    float v = vorticity[DTid.xy].x;
    
    float2 eta = float2(abs((vr - vl) / dx), abs((vt - vb) / dy));
    float l = length(eta);
    if(l<1e-5)
        return;
    
    float3 N = float3(normalize(eta), 0.f);
    float3 w = float3(0.f, 0.f, v);
   
   
    velocity[DTid.xy].xy += gConstantBuffer.vorticity * cross(N, w).xy * 0.002f;
}