#include "Particle.hlsli"

RWStructuredBuffer<Particle> particles : register(u0);
struct SimulationConstant
{
    float delTime;
};
ConstantBuffer<SimulationConstant> gConstantBuffer : register(b0);
[numthreads(768, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    Particle p = particles[DTid.x];
    
    //if (p.life <= 0.f)
    //{
    //    p.position = p.originPosition;
    //    p.velocity = p.originVelocity;
    //}
    
    float3 position = p.position;
    float3 a = float3(0, -9.8f, 0.f);
    p.velocity += a.xy * gConstantBuffer.delTime;
    
    
    float f = 0.5f;
    
    if (p.position.y <= -1.f + p.radius)
    {
        p.velocity.y *= -f;
    }
    if (p.position.x <= -1.f + p.radius || p.position.x >= 1.f - p.radius)
    {
        p.velocity.x *= -f;
    }
    p.position.xy += p.velocity * gConstantBuffer.delTime;
    p.life -= 1.f * gConstantBuffer.delTime;
    
   
    
    particles[DTid.x] = p;

}