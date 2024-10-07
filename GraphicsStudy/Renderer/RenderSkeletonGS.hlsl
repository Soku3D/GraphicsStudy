#include "RenderSkeleton.hlsli"

struct GSOutput
{
    float4 pos : SV_POSITION;
};

[maxvertexcount(3)]
void main(
	point float4 input[1] : POSITION,
	inout TriangleStream<GSOutput> output
)
{
    float l = 0.01f;
    float2 offset[3] =
    {
        float2(-l, -l),
        float2(0, l),
        float2(l, -l)
    };
    
    for (uint i = 0; i < 3; i++)
    {
        GSOutput element;
        float4 pos = float4(input[0].xyz, 1) + float4(offset[i], 0, 0);
        float4 svPosition = mul(pos, View);
        svPosition = mul(svPosition, Projection);
        element.pos = svPosition;
        output.Append(element);
    }
    output.RestartStrip();
}