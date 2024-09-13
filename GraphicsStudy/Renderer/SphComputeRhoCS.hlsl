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
    float rho = 0.f;
    float h = p.radius;
    float hd = pow(h,2);
    
    for(uint i = 0; i<SIMULATION_PARTICLE_SIZE; ++i){
        Particle pJ = particles[i];

        rho+=ComputeWeight(p.position, pJ.position, h, hd);
    }

    p.rho = rho;

    float k = 1.f;
    float rho0 = 1.f;

    p.pressure = k*(pow(p.rho / rho0, 7.f) - 1.f);
    particles[DTid.x] = p;

}