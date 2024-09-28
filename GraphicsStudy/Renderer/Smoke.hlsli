
struct SimulationConstant
{
    float3 color;
    float deltaTime;
    float3 velocity;
    float radius;
    
    float viscosity;
    float vorticity;
    uint i;
    uint j;
    
    float time;
    float sourceStrength;
};

SamplerState gWarpLinearSampler : register(s0);
SamplerState gWarpPointSampler : register(s1);
SamplerState gClampLinearSampler : register(s2);
SamplerState gClampPointSampler : register(s3);

ConstantBuffer<SimulationConstant> gConstantBuffer : register(b0);
