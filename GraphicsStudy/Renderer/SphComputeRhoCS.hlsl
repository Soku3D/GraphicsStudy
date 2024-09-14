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
        return;
    
    p.rho = 0.f;
    float h = p.radius;

    for (uint i = 0; i < SIMULATION_PARTICLE_SIZE; ++i)
    {
        Particle pJ = particles[i];
        if(pJ.life < 0.f)
            continue;
       
        float3 xij = p.position - pJ.position;
        float dist = length(xij);
         
        if (dist >= p.radius)
            continue;
        
        p.rho += pJ.mass * ComputeWeight(xij, h);
    }
    
    float k = 1.f;
    float rho0 = 1.f;
    p.pressure = k * (pow(p.rho / rho0, 7.f) - 1.f);
    
    particles[DTid.x] = p;

}