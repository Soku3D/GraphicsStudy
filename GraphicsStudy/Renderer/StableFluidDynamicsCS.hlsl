#include "CFD.hlsli"

RWTexture2D<float4> density : register(u0);
RWTexture2D<float4> velocity : register(u1);
struct SimulationConstant
{
    float3 color;
    
    uint i;
    uint j;
};
ConstantBuffer<SimulationConstant> gConstantBuffer : register(b0);

[numthreads(32, 32, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    uint width, height;
    density.GetDimensions(width, height);
    
    float radius = 10.f;
    
    if (gConstantBuffer.i < width)
    {
        float2 mousePos = float2(gConstantBuffer.i, gConstantBuffer.j);
        
        float dist = length(mousePos - DTid.xy);
        if (dist < radius)
        {
            density[DTid.xy] = float4(gConstantBuffer.color, 1.f);
        }
    }
}