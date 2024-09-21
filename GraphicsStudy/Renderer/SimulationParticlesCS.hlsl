#include "Particle.hlsli"

RWStructuredBuffer<Particle> particles : register(u0);
Texture2D density : register(t0);

static const float deltaTime = 1.f / 500.f;

struct SimulationConstant
{
    float delTime;
};
ConstantBuffer<SimulationConstant> gConstantBuffer : register(b0);
[numthreads(768, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    Particle p = particles[DTid.x];
        
    float3 position = p.position;
    float3 direction = cross(normalize(position), float3(0, 0, -1));
    float velocity = 0.5f;
    p.position += direction * gConstantBuffer.delTime * velocity;
    
    particles[DTid.x] = p;

}