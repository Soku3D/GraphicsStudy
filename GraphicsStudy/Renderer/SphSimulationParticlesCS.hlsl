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
            p.life = 0.5f;
            p.position = p.originPosition;
            p.velocity = p.originVelocity;
        }
    }
    else
    {
        float3 position = p.position;
    
        p.velocity += (p.force / p.mass) * gConstantBuffer.delTime;
        p.position += p.velocity * gConstantBuffer.delTime;
    
        float f = 0.5f;
        
        float xStart = -0.9f;
        float xEnd = 0.9f;
        float yEnd = -0.9f;
        
        // 벽면 충돌
        if (p.position.y <= yEnd)
        {
            p.position.y = yEnd;
            p.velocity.y *= -f;
        }
        if (p.position.x <= xStart || p.position.x >= xEnd)
        {
            if (p.position.x <= xStart)
                p.position.x = xStart;
            if (p.position.x >= xEnd)
                p.position.x = xEnd;
        
            p.velocity.x *= -f;
        }
        
        // 속도에 따른 색
        //if (length(p.velocity) > 0.1f)
        //{
        //    p.color = float3(1, 1, 1);
        //}
        //else
        //{
        //    p.color = float3(0, 0, 1);
        //}
    }
      
    particles[DTid.x] = p;

}