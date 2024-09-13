#include "Particle.hlsli"

StructuredBuffer<Particle> particles : register(t0);
struct VSInput
{
    uint vId : SV_VertexID;
};

struct GSInput
{
    float3 position : Position;
    float3 color : COLOR;
    float radius : DEPTH0;
    float life : DEPTH1;
};
struct PSInput
{
    float4 svPosition : SV_Position;
    float2 position : Position0;
    float2 center : Position1;
    float radius : DEPTH0;
    float3 color : COLOR;
    float life : DEPTH1;
};