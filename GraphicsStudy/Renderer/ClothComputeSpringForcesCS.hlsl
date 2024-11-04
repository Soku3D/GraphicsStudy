#include "Particle.hlsli"

RWStructuredBuffer<Particle> particles : register(u0);
StructuredBuffer<Particle> particlesTemp : register(t0);
static const int height = 40;
static const int width = 40;
static const float3 windvel = float3(0.01f, 0.f, -0.005f);
static const float gravity = 0.0022f;

struct SimulationConstant
{
    float delTime;
};

ConstantBuffer<SimulationConstant> gConstantBuffer : register(b0);



float2 GetPos(int pos)
{
    return float2(pos / height, pos % height);
}

float3 ComputeEdge(float2 dir, int uId, float3 position, float3 velocity)
{
    float2 Xi = GetPos(uId);
    float2 Xj = Xi + dir;
    float3 vel = float3(0.f, 0.f, 0.f);
    if (Xj.x >= 0.f && Xj.y >= 0.f && Xj.x < width && Xj.y < height)
    {
        uint posJ = Xj.x + Xj.y * height;
        Particle Pj = particlesTemp[posJ];
        
        float edgelen = length(dir);
        float3 posdif = Pj.position - position;
        float3 veldif = Pj.velocity - velocity;

        vel += normalize(posdif) * (clamp(length(posdif) - edgelen, -1.0, 1.0) * 0.015); // spring

        vel += normalize(posdif) * (dot(normalize(posdif), veldif) * 0.010); // damper
    }
    return vel;
}


[numthreads(768, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    Particle p = particlesTemp[DTid.x];
    
    float l = 1.f;
    float3 velocity = p.velocity;
    float3 position = p.position;
    
    velocity += ComputeEdge(float2(0.0, l), DTid.x, position, p.velocity);
    velocity += ComputeEdge(float2(0.0, -l), DTid.x, position, p.velocity);
    velocity += ComputeEdge(float2(l, 0.0), DTid.x, position, p.velocity);
    velocity += ComputeEdge(float2(-l, 0.0), DTid.x, position, p.velocity);
    velocity += ComputeEdge(float2(l, l), DTid.x, position, p.velocity);
    velocity += ComputeEdge(float2(-l, -l), DTid.x, position, p.velocity);
    
    l = 2.f;
    velocity += ComputeEdge(float2(0.0, l), DTid.x, position, p.velocity);
    velocity += ComputeEdge(float2(0.0, -l), DTid.x, position, p.velocity);
    velocity += ComputeEdge(float2(l, 0.0), DTid.x, position, p.velocity);
    velocity += ComputeEdge(float2(-l, 0.0), DTid.x, position, p.velocity);
    velocity += ComputeEdge(float2(l, l), DTid.x, position, p.velocity);
    velocity += ComputeEdge(float2(-l, -l), DTid.x, position, p.velocity);
    
    p.velocity = velocity;
    //p.position = position + velocity;
    //p.position += velocity;
    p.velocity.y -= gravity; 
    
    float baseX = -2.f;
    float baseY = 2.f;
    float n = 40.f;
    float dx = 4.f / n;
    float dy = 4.f / n;
    
    int y = DTid.x / height;
    if (y == 0)
    {
        p.position = float3(baseX + dx * DTid.x * 0.9f, baseY, 0.f);
    }
    
    particles[DTid.x] = p;
}