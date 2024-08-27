struct Particle
{
    float3 position;
    float3 color;
};
RWStructuredBuffer<Particle> particles : register(u0);

static const float deltaTime = 1.f / 300.f;

[numthreads(768, 1, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
    Particle p = particles[DTid.x];
    float3 position = p.position;
    float3 direction = cross(normalize(position), float3(0, 0, -1));
    p.position += direction * deltaTime;
    
    particles[DTid.x] = p;

}