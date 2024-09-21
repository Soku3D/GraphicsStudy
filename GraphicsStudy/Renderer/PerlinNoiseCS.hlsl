#include "Noise.hlsli"

RWTexture2D<float4> gTexture : register(u0);
struct SimulationConstant
{
    float delTime;
    float time;
};
ConstantBuffer<SimulationConstant> gConstantBuffer : register(b0);

float getNoise(float2 uv)
{
    return perlinfbm(float3(uv, 0.0f), 2.0, 1);
}



[numthreads(32, 32, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    uint width, height;
    gTexture.GetDimensions(width, height);
    uint x = DTid.x;
    uint y = DTid.y;

    float2 uv = (float2(x, y) + 0.5f) / float2(width, height);
    uv = uv * 2.f - 1.f;
    //float val = 0.5f + 0.5f * Pseudo3dNoise(float3(uv* 2.f, gConstantBuffer.time));
    //float3 color = float3(val, val, val);
    
    //gTexture[DTid.xy] = float4(color, 1.f);
    //const int ITERATIONS = 10;
    //float noiseVal = 0.0;
    //float sum = 0.0;
    //float multiplier = 1.0;
    //for (int i = 0; i < ITERATIONS; i++)
    //{
    //    float3 noisePos = float3(uv* 10.f, 0.2 * gConstantBuffer.time / multiplier);
    //    noiseVal += multiplier * abs(Pseudo3dNoise(noisePos));
    //    sum += multiplier;
    //    multiplier *= 0.6;
    //    uv = 2.0 * uv + 4.3;
    //}
    //noiseVal /= sum;
    //color = 0.5 + 0.5 * cos(6.283185 * (3.0 * noiseVal + float3(0.15, 0.0, 0.0)));
    
    
    //gTexture[DTid.xy] = float4(color, 1.f);
    float3 color;
    color.rgb = getNoise(uv);
    
    gTexture[DTid.xy] = float4(color, 1.f);
}
