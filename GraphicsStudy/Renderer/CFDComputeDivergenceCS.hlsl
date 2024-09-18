#include "CFD.hlsli"
#include "Utility.hlsli"

Texture2D velocity : register(s0);
RWTexture2D<float4> Divergence : register(u0);
RWTexture2D<float4> pressure : register(u1);
RWTexture2D<float4> pressureTemp : register(u2);

SamplerState gWarpLinearSampler : register(s0);
SamplerState gWarpPointSampler : register(s1);

ConstantBuffer<SimulationConstant> gConstantBuffer : register(b0);

[numthreads(32, 32, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    uint width, height;
    Divergence.GetDimensions(width, height);
    uint x = DTid.x;
    uint y = DTid.y;
    
    float dx = 2.f / width;
    float dy = 2.f / height;
    
    dx = 2.f;
    dy = 2.f;
    
    uint2 left = uint2(((x - 1 < 0) ? (width - 1) : (x - 1)), y);
    uint2 right = uint2(((x + 1 > width - 1) ? (0) : (x + 1)), y);
    uint2 top = uint2(x, ((y - 1 < 0) ? (height - 1) : (y - 1)));
    uint2 bottom = uint2(x, ((y + 1 > height - 1) ? (0) : (y + 1)));
    
    float vl = velocity[left].x; 
    float vr = velocity[right].x;
    float vb = velocity[top].y; 
    float vt = velocity[bottom].y; 
    
    float divergence = (vr - vl) / dx + (vt - vb) / dy;
    
    Divergence[DTid.xy].x = divergence;
    pressure[DTid.xy].x = 0.f;
    pressureTemp[DTid.xy].x = 0.f;

}