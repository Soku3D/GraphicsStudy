#include "CFD.hlsli"
#include "Utility.hlsli"

RWTexture2D<float4> density : register(u0);
RWTexture2D<float4> velocity : register(u1);

Texture2D densityTemp : register(s0);
Texture2D velocityTemp : register(s1);

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
    
    
    uint width, height;
    density.GetDimensions(width, height);
    
    float x = (DTid.x + 0.5f) / width;
    float y = (DTid.y + 0.5f) / height;
    float2 currUVPosition = float2(x, y);
    
    float2 vel = velocityTemp.SampleLevel(gWarpPointSampler, currUVPosition, 0.f).xy;
    
    float2 position = currUVPosition - vel * gConstantBuffer.deltaTime;
    density[DTid.xy] = densityTemp.SampleLevel(gWarpLinearSampler, position, 0.f);
    velocity[DTid.xy] = velocityTemp.SampleLevel(gWarpLinearSampler, position, 0.f);
    
    //uint width, height;
    //velocity.GetDimensions(width, height);
    //float2 dx = float2(1.0 / width, 1.0 / height);
    //float2 pos = (DTid.xy + 0.5) * dx; // �� �����尡 ó���ϴ� ���� �߽�
    
    //float2 vel = velocityTemp.SampleLevel(gWarpPointSampler, pos, 0.f).xy;
    
    //float2 posBack = pos - vel * gConstantBuffer.deltaTime;

    //velocity[DTid.xy] = velocityTemp.SampleLevel(gWarpLinearSampler, posBack, 0.f);
    //density[DTid.xy] = densityTemp.SampleLevel(gWarpLinearSampler, posBack, 0.f);
}

