cbuffer cbPass : register(b0)
{
    float4x4 boneTransforms[60];
    float4x4 baseTransforms[60];
}
cbuffer cbPass : register(b1)
{
    matrix View;
    matrix Projection;
    float3 eyePosition;
}
struct VSInput
{
    float3 position : POSITION;
    uint vId : SV_VertexID;
};

struct GSInput
{
    float3 position : POSITION;
};
struct PSInput
{
    float4 sv_position : SV_POSITION;
};