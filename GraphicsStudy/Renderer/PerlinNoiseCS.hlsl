RWTexture2D<float4> gTexture : register(u0);
struct SimulationConstant
{
    float delTime;
    float time;
};
ConstantBuffer<SimulationConstant> gConstantBuffer : register(b0);

float2 GetGradient(float2 intPos, float t)
{
    
    // Uncomment for calculated rand
    float rand = frac(sin(dot(intPos, float2(12.9898, 78.233))) * 43758.5453);;
    
    float angle = 6.283185 * rand + 4.0 * t * rand;
    return float2(sin(angle),  cos(angle) );
}

float Pseudo3dNoise(float3 pos)
{
    float2 i = floor(pos.xy);
    float2 f = pos.xy - i;
    float2 blend = f * f * (3.0 - 2.0 * f);
    float noiseVal =
        lerp(
            lerp(
                dot(GetGradient(i + float2(0, 0), pos.z), f - float2(0, 0)),
                dot(GetGradient(i + float2(1, 0), pos.z), f - float2(1, 0)),
                blend.x),
            lerp(
                dot(GetGradient(i + float2(0, 1), pos.z), f - float2(0, 1)),
                dot(GetGradient(i + float2(1, 1), pos.z), f - float2(1, 1)),
                blend.x),
        blend.y
    );
    return noiseVal; // normalize to about [-1..1]
}

[numthreads(32, 32, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    uint width, height;
    gTexture.GetDimensions(width, height);
    uint x = DTid.x;
    uint y = DTid.y;

    float2 uv = (float2(x, y) + 0.5f) / float2(width, height);
    //float2 uv = float2(x, y) / float2(width - 1.f, height - 1.f);
    float val = 0.5f + 0.5f * Pseudo3dNoise(float3(uv * 10, 1.f));
    float3 color = float3(val, val, val);
    
    gTexture[DTid.xy] = float4(color, 1.f);
}
