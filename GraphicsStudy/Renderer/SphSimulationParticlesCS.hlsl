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
    
    if (p.life < 0.f)
    {
        p.life += gConstantBuffer.delTime;
        if (p.life > 0.f)
        {
            p.life = 1.f;
            p.position = p.originPosition;
            p.currVelocity = p.originVelocity;
            p.prevVelocity = p.originVelocity;
        }
    }
    else
    {
        float3 position = p.position;
    
        p.currVelocity += (p.force / p.mass) * gConstantBuffer.delTime;
        p.position += p.currVelocity * gConstantBuffer.delTime;
        p.prevVelocity = p.currVelocity;
    
        float f = 0.5f;
    
        if (p.position.y <= -1.f + p.radius)
        {
            p.position.y = -1.f + p.radius;
            p.currVelocity.y *= -f;
        }
        if (p.position.x <= -1.f + p.radius || p.position.x >= 1.f - p.radius)
        {
            if (p.position.x <= -1.f + p.radius)
                p.position.x = -1.f + p.radius;
            if (p.position.x >= 1.f - p.radius)
                p.position.x = 1.f - p.radius;
        
            p.currVelocity.x *= -f;
        }
    }
      
    particles[DTid.x] = p;

}