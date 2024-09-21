#include "Particle.hlsli"
#include "Noise.hlsli"

RWStructuredBuffer<Particle> particles : register(u0);
Texture2D random : register(t0);

struct SimulationConstant
{
    float deltaTime;
};
ConstantBuffer<SimulationConstant> gConstantBuffer : register(b0);
float getNoise(float2 uv)
{
    //return perlinfbm(float3(uv, 0.0f), 2.0, 1);
    return 0.5f + 0.5f * Pseudo3dNoise(float3(uv * 2.f, 1.f));

}

float2 getCurl(float2 uv, float2 dx)
{
    float2 dp;
    dp.x = (getNoise(uv + float2(dx.x, 0)) - getNoise(uv - float2(dx.x, 0))) / dx.x * 0.5f;
    dp.y = (getNoise(uv + float2(0, dx.y)) - getNoise(uv - float2(0, dx.y))) / dx.y * 0.5f;
    
    return float2(dp.y, -dp.x);
}

[numthreads(768, 1, 1)]
void main(uint3 dtID : SV_DispatchThreadID, uint gIndex : SV_GroupIndex)
{
    uint width, height;
    random.GetDimensions(width, height);
    Particle p = particles[dtID.x];
    
    float2 dx = float2(1.0 / width, 1.0 / height);
    
    uint x = ((p.position.x + 1.f) / 2.f) * (width - 1);
    uint y = (-(p.position.y - 1.f) / 2.f) * (height - 1);
    
    uint2 left = uint2(((x == 0) ? (width - 1) : (x - 1)), y);
    uint2 right = uint2(((x == width - 1) ? (0) : (x + 1)), y);
    uint2 top = uint2(x, ((y == 0) ? (height - 1) : (y - 1)));
    uint2 bottom = uint2(x, ((y == height - 1) ? (0) : (y + 1)));
    
    float rl = random[left].x;
    float rr = random[right].x;
    float rb = random[top].x;
    float rt = random[bottom].x;
    float2 uv = p.position.xy;
    
    float2 dp;
    dp.x = (getNoise(uv + float2(dx.x, 0)) - getNoise(uv - float2(dx.x, 0))) / dx.x * 0.5f;
    dp.y = (getNoise(uv + float2(0, dx.y)) - getNoise(uv - float2(0, dx.y))) / dx.y * 0.5f;
    float2 culRandom = float2(dp.y, -dp.x);
    
    particles[dtID.x].position.xy += culRandom* gConstantBuffer.deltaTime;
    //outputParticles[dtID.x].position.xy = float2(0, 0);
}
