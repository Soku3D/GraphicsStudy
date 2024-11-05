#include "Particle.hlsli"

RWStructuredBuffer<Particle> particles : register(u0);
StructuredBuffer<Particle> particlesTemp : register(t0);

static const float3 windvel = float3(0.01f, 0.f, -0.005f);
static const float gravity = 0.0022f;

struct SimulationConstant
{
    float delTime;
    float width;
    float height;
};

ConstantBuffer<SimulationConstant> gConstantBuffer : register(b0);

float2 GetPos(int pos)
{
    return float2(pos % (int) gConstantBuffer.height, pos / (int) gConstantBuffer.height);
}

float3 GetNormal(int pos)
{
    float2 currPos = GetPos(pos);
    int width = (int) gConstantBuffer.width;
    int height = (int) gConstantBuffer.height;
    
    int l = currPos.x > 0 ? pos - 1 : pos;
    int r = currPos.x < width-1 ? pos + 1 : pos;
    int u = currPos.y > 0 ? pos - height : pos;
    int d = currPos.y < height-1 ? pos + height : pos;
    
    float3 pl = particlesTemp[l].position;
    float3 pr = particlesTemp[r].position;
    float3 pu = particlesTemp[u].position;
    float3 pd = particlesTemp[d].position;
    
    return cross(pu - pd, pr - pl);
}


float3 ComputeEdge(float2 dir, int uId, float3 position, float3 velocity)
{
    int width = (int) gConstantBuffer.width;
    int height = (int) gConstantBuffer.height;
    
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
        
        vel += normalize(posdif) * (clamp(length(posdif) - edgelen, -1.0, 1.0) * 0.15) * 0.5f; // spring

        vel += normalize(posdif) * (dot(normalize(posdif), veldif) * 0.10) * 0.5f; // damper
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
    
    int width = (int) gConstantBuffer.width;
    int height = (int) gConstantBuffer.height;
    
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
    velocity += ComputeEdge(float2(l, -l), DTid.x, position, p.velocity);
    velocity += ComputeEdge(float2(-l, l), DTid.x, position, p.velocity);
    
    p.velocity = velocity;
    
    p.position = position + velocity;
    p.velocity.y -= gravity; 
    
    float3 norm = normalize(GetNormal(DTid.x));
    //velocity += norm * (dot(norm, velocity - windvel) * 0.05);
    
    
    int y = DTid.x / height;
    if (y == 0)
    {
        float baseX = 0.f;
        float baseY = 0.f;
        float dx = 1.f;
        float dy = 1.f;

        p.position = float3(baseX + dx * (DTid.x % height) * 0.85f, baseY, 0.f);
        p.velocity = float3(0.f, 0.f, 0.f);
    }
    
    particles[DTid.x] = p;
}