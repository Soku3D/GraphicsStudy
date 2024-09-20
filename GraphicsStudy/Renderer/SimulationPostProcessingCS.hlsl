RWTexture2D<float4> gOutput : register(u0);

struct SimulationConstant
{
    float delTime;
    float time;
};
ConstantBuffer<SimulationConstant> gConstantBuffer : register(b0);

[numthreads(32, 32, 1)]
void main(int3 gID : SV_GroupID, uint3 DTid : SV_DispatchThreadID)
{
    float3 color = gOutput[DTid.xy].rgb;
    color = max(0.f, color - 10.f * gConstantBuffer.delTime);
    
    gOutput[DTid.xy] = float4(color, 1.f);
}