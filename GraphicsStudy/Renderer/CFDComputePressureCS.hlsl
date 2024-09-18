#include "CFD.hlsli"
#include "Utility.hlsli"

RWTexture2D<float4> pressure : register(u0);
Texture2D pressureTemp : register(t0);
Texture2D divergence : register(t1);

SamplerState gWarpLinearSampler : register(s0);
SamplerState gWarpPointSampler : register(s1);

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
    if (DTid.x == 0 && DTid.y == 0)
    {
        pressure[DTid.xy] = 0.0;
        return;
    }
    uint width, height;
    divergence.GetDimensions(width, height);
    uint x = DTid.x;
    uint y = DTid.y;
    
    float dx = 1.f;
    float dy = 1.f;
    
    uint2 left = uint2(((x - 1 < 0) ? (width - 1) : (x - 1)), y);
    uint2 right = uint2(((x + 1 > width - 1) ? (0) : (x + 1)), y);
    uint2 top = uint2(x, ((y - 1 < 0) ? (height - 1) : (y - 1)));
    uint2 bottom = uint2(x, ((y + 1 > height - 1) ? (0) : (y + 1)));
    
    float div = divergence.SampleLevel(gWarpPointSampler, GetTexcoord(width, height, float2(DTid.xy)), 0.f).x;
    float pressureSum = 0.f;
    pressureSum += pressureTemp.SampleLevel(gWarpPointSampler, GetTexcoord(width, height, float2(left)), 0.f).x;
    pressureSum += pressureTemp.SampleLevel(gWarpPointSampler, GetTexcoord(width, height, float2(right)), 0.f).x;
    pressureSum += pressureTemp.SampleLevel(gWarpPointSampler, GetTexcoord(width, height, float2(top)), 0.f).x;
    pressureSum += pressureTemp.SampleLevel(gWarpPointSampler, GetTexcoord(width, height, float2(bottom)), 0.f).x;
    
    pressure[DTid.xy] = (pressureSum - div) / 4.f;

}