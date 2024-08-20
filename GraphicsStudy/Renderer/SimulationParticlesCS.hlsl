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
    //p.position += float3(0.f, -2.f * deltaTime ,0.f);
    if (p.position.y < -1.f)
    {
        p.position.y = 1.1f;
    }
    particles[DTid.x] = p;

}