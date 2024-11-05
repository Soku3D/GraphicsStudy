#include "Particle.hlsli"

StructuredBuffer<Particle> particles : register(t0);
cbuffer cbPass : register(b0)
{
    matrix View;
    matrix Projection;
    float3 eyePosition;
}
cbuffer SimulationConstant : register(b1)
{
    float delTime;
    float width;
    float height;
}
struct VSInput
{
    uint vId : SV_VertexID;
};

struct GSInput
{
    float3 position0 : Position0;
    float3 position1 : Position1;
    float3 position2 : Position2;
    float3 position3 : Position3;
};
struct PSInput
{
    float4 svPosition : SV_Position;
    float3 normal : NORMAL;
};