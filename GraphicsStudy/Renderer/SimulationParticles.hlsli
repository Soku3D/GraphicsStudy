struct Particle
{
    float3 position;
    float3 color;
};
StructuredBuffer<Particle> particles : register(t0);
struct VSInput
{
    uint vId : SV_VertexID;
};

struct GSInput
{
    float3 position : Position;
    float3 color : COLOR;
};
struct PSInput
{
    float4 svPosition : SV_Position;
    float3 color : COLOR;
};