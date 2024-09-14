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
    
    float3 gravity = p.mass * float3(0, -9.8f, 0.f);
    float3 viscosityForce = float3(0.f, 0.f, 0.f);
    float3 pressureForce = float3(0.f, 0.f, 0.f);
    
    float h = p.radius;
    float m = p.mass;
    
    float3 x_i = p.position;
    float rho_i = p.density;
    
    for (uint i = 0; i < SPH_SIMULATION_PARTICLE_SIZE; ++i)
    {
        Particle pJ = particles[i];
        
        if (pJ.life < 0.0f)
            continue;

        if (i == DTid.x)
            continue;

        float3 x_j = pJ.position;
        
        float3 x_ij = x_i - x_j;
        
        float dist = length(x_ij);
        
        if (dist >= pJ.radius)
            continue;
        
        if (dist < 1e-3f) 
            continue;
        
        viscosityForce += p.viscosity * LaplaceOperator(x_ij, p.density, pJ.density,
            p.velocity, pJ.velocity, h, m, dist);
        
        pressureForce -= Del(x_ij, p.density, pJ.density,
            p.pressure, pJ.pressure, h, m, dist) / p.density;
    }
    p.force = gravity + pressureForce + viscosityForce;
    particles[DTid.x] = p;
}