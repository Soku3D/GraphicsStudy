#include "CFD.hlsli"
#include "Utility.hlsli"

RWTexture2D<float4> vorticity : register(u0);

Texture2D velocity : register(t0);

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
    
    dx = 2.f;
    dy = 2.f;
    
    //uint2 left = uint2(((x == 0) ? (width - 1) : (x - 1)), y);
    //uint2 right = uint2(((x == width - 1) ? (0) : (x + 1)), y);
    //uint2 top = uint2(x, ((y == 0) ? (height - 1) : (y - 1)));
    //uint2 bottom = uint2(x, ((y == height - 1) ? (0) : (y + 1)));
    
      
    uint2 left = uint2(((x == 0) ? 0 : (x - 1)), y);
    uint2 right = uint2(((x == width - 1) ? width - 1 : (x + 1)), y);
    uint2 top = uint2(x, ((y == 0) ? 0 : (y - 1)));
    uint2 bottom = uint2(x, ((y == height - 1) ? height - 1 : (y + 1)));
    
    float vl = velocity[left].y;
    float vr = velocity[right].y;
    float vt = velocity[top].x;
    float vb = velocity[bottom].x;

    vorticity[DTid.xy].x = (vr - vl) / dy - (vt - vb) / dx;

}