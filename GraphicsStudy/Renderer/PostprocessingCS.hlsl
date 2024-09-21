RWTexture2D<float4> gOutput : register(u0);

static const float deltaTime = 1.f / 300.f;
struct postProcessingConstant
{
    bool bUseGamma;
};
ConstantBuffer<postProcessingConstant> gConstantBuffer : register(b0);
[numthreads(32, 32, 1)]
void main(int3 gID : SV_GroupID, uint3 DTid : SV_DispatchThreadID)
{
    float gamma = 2.2f;
    float invGamma = 1 / 2.2f;
    float3 color;
    if (gConstantBuffer.bUseGamma)
    {
        color = pow(gOutput[DTid.xy].rgb, gamma);
    }
    else
    {
        color = pow(gOutput[DTid.xy].rgb, invGamma);
    }
    //color -= 3.f * deltaTime;
    gOutput[DTid.xy] = float4(color, 1.f);
  
}