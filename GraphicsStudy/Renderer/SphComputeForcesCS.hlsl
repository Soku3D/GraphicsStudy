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
    float v = 1.f;
    float m = p.mass;
    
    float rho_i = p.rho;
    float p_i = p.pressure;
    float3 x_i = p.position;
    float3 v_i = p.currVelocity;
    
    for (uint i = 0; i < SIMULATION_PARTICLE_SIZE; ++i)
    {
        Particle pJ = particles[i];
        if (pJ.life < 0.0f)
            continue;

        if (i == DTid.x)
            continue;

      
        float3 x_j = pJ.position;
        
        float rho_j = pJ.rho;
        float p_j = pJ.pressure;
        float3 x_ij = x_i - x_j;
        float3 v_j = pJ.currVelocity;

        float dist = length(x_i - x_j);
        float m_mass = pJ.mass;
        float m_radius = pJ.radius;
        
        if (dist >= pJ.radius)
            continue;

        if (dist < 1e-3f) 
            continue;
        
        const float3 gradPressure =
                rho_i * m_mass *
                (p_i / (rho_i * rho_i) + p_j / (rho_j * rho_j)) *
                DelKernel(dist * 2.f / m_radius) *
                (x_i - x_j) / dist;

        const float3 laplacianVelocity =
                2.f * m_mass / rho_j * (v_i - v_j) /
                (dot(x_ij, x_ij) + 0.01f * m_radius * m_radius) *
                DelKernel(dist * 2.f / m_radius) *
                dot(x_ij,x_ij / dist);

        pressureForce -= m_mass / rho_i * gradPressure;
        viscosityForce += m_mass * 0.1f * laplacianVelocity;
        
    }
    p.force = gravity + pressureForce + viscosityForce;
    particles[DTid.x] = p;
}