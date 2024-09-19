#include "CFD.hlsli"
#include "Utility.hlsli"

RWTexture2D<float4> velocity : register(u0);
Texture2D<float4> pressure : register(s0);

ConstantBuffer<SimulationConstant> gConstantBuffer : register(b0);

SamplerState gWarpLinearSampler : register(s0);
SamplerState gWarpPointSampler : register(s1);

[numthreads(32, 32, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    uint width, height;
    velocity.GetDimensions(width, height);
    uint x = DTid.x;
    uint y = DTid.y;
    
    float dx = 2.f;
    float dy = 2.f;
       
    uint2 left = uint2(((x == 0) ? (width - 1) : (x - 1)), y);
    uint2 right = uint2(((x == width - 1) ? (0) : (x + 1)), y);
    uint2 top = uint2(x, ((y  == 0) ? (height - 1) : (y - 1)));
    uint2 bottom = uint2(x, ((y  == height - 1) ? (0) : (y + 1)));
    
    
    float pl = pressure[left].x; 
    float pr = pressure[right].x;
    float pb = pressure[top].y;
    float pt = pressure[bottom].y;

    float2 divergence = float2((pr - pl) / dx, (pt - pb) / dy);
    
    velocity[DTid.xy].xy = velocity[DTid.xy].xy - divergence;

}