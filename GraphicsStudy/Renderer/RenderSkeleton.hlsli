cbuffer cbSkeleton : register(b0)
{
    float4x4 boneTransforms[60];
    float4x4 baseTransforms[60];
    float4 parentsIndex[15];
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
    float3 position : POSITION0;
    float3 parentsPosition : POSITION1;
};
struct PSInput
{
    float4 sv_position : SV_POSITION;
    float3 normal : NORMAL;
};