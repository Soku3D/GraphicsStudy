#include "RenderSkeleton.hlsli"

[maxvertexcount(3)]
void main(
	point GSInput input[1],
	inout LineStream<PSInput> output
)
{
    //float l = 0.01f;
    //float2 offset[3] =
    //{
    //    float2(-l, -l),
    //    float2(0, l),
    //    float2(l, -l)
    //};
    //for (int i = 0; i < 3; i++)
    //{
    //    PSInput element;
    //    float4 pos = float4(input[0].position, 1.f) + float4(offset[i], 0, 0);
    //    pos = mul(pos, View);
    //    pos = mul(pos, Projection);
    //    element.sv_position = pos;
        
    //    output.Append(element);
    //}
    
    PSInput element;
    float4 pos1 = float4(input[0].position, 1.f);
    pos1 = mul(pos1, View);
    pos1 = mul(pos1, Projection);
    element.sv_position = pos1;
    element.texcoord = float2(1, 0);
    output.Append(element);
    
    float4 pos2 = float4(input[0].parentsPosition, 1.f);
    pos2 = mul(pos2, View);
    pos2 = mul(pos2, Projection);
    element.sv_position = pos2;
    element.texcoord = float2(0, 0);
    output.Append(element);
        
    output.RestartStrip();
        
 
}