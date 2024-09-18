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
    
    //float d = dencityTemp[DTid.xy];
    //float dl = dencityTemp[left];
    //float dr = dencityTemp[right];
    //float db = dencityTemp[top].x;
    //float dt = dencityTemp[bottom].x;
    //float laplaceVel = dl + dr + db + dt - 5.f * d;
        
    dencity[DTid.xy] = dencityTemp[DTid.xy]  +
        gConstantBuffer.deltaTime * gConstantBuffer.viscosity *
                (dencityTemp[left] +
                dencityTemp[right] +
                dencityTemp[top] +
                dencityTemp[bottom] - 4.f * dencityTemp[DTid.xy]);

}