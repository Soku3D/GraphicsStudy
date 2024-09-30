
struct VSInput
{
    float3 position : POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD;
    float3 tangent : TANGENT;
};

struct PSInput
{
    float4 ClipPos : SV_POSITION; // Clip space position for rasterization
    float2 TexCoord : TEXCOORD; // Pass-through texture coordinates
    float4 PrevClipPos : TEXCOORD1; // Clip space position from the previous frame
    float3 WorldPos : TEXCOORD2; // World position for motion vector computation
};
