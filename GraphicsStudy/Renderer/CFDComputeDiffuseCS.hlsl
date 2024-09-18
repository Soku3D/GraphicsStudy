#include "CFD.hlsli"
#include "Utility.hlsli"

RWTexture2D<float4> dencity : register(u0);
RWTexture2D<float4> velocity : register(u1);

Texture2D dencityTemp : register(t0);
Texture2D velocityTemp : register(t1);

SamplerState gWarpLinearSampler : register(s0);
SamplerState gWarpPointSampler : register(s1);

ConstantBuffer<SimulationConstant> gConstantBuffer : register(b0);

[numthreads(32, 32, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    uint width, height;
    dencity.GetDimensions(width, height);
    uint x = DTid.x;
    uint y = DTid.y;
    
    uint2 left = uint2(((x - 1 < 0) ? (width - 1) : (x - 1)), y);
    uint2 right = uint2(((x + 1 > width - 1) ? (0) : (x + 1)), y);
    uint2 top = uint2(x, ((y - 1 < 0) ? (height - 1) : (y - 1)));
    uint2 bottom = uint2(x, ((y + 1 > height - 1) ? (0) : (y + 1)));
    
    float4 d = dencityTemp[DTid.xy];
    float4 dl = dencityTemp[left];
    float4 dr = dencityTemp[right];
    float4 db = dencityTemp[top];
    float4 dt = dencityTemp[bottom];
    
    //float laplaceVel = dl + dr + db + dt - 5.f * d;
    float kd = gConstantBuffer.viscosity * gConstantBuffer.deltaTime;
    
    
    dencity[DTid.xy] = (d + kd * (dl + dr + db + dt)) / (1 + 4.f * kd);
}