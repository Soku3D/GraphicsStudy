#include "DrawNormalPass.hlsli"

[maxvertexcount(6)]
void main(
	point GSInput input[1],
	inout LineStream<PSInput> output
)
{
    PSInput p0;
    p0.worldPoition = input[0].worldPoition;
    float4 position = float4(p0.worldPoition, 1.f);
    position = mul(position, View);
    position = mul(position, Projection);
    p0.svPosition = position;
    p0.coordinate = float3(0.f, 0.f, 0.f);
    output.Append(p0);
    
    PSInput p1;
    p1.worldPoition = p0.worldPoition + 0.02f * input[0].normal;
    position = float4(p1.worldPoition, 1.f);
    position = mul(position, View);
    position = mul(position, Projection);
    p1.svPosition = position;
    p1.coordinate = float3(1.f, 0.f, 0.f);
    output.Append(p1);
    
    output.RestartStrip();
    
    p0.coordinate = float3(0.f, 1.f, 0.f);
    output.Append(p0);
    
    PSInput p2;
    p2.worldPoition = p0.worldPoition + 0.02f * input[0].tangent;
    position = float4(p2.worldPoition, 1.f);
    position = mul(position, View);
    position = mul(position, Projection);
    p2.svPosition = position;
    p2.coordinate = float3(1.f, 1.f, 0.f);
    output.Append(p2);
    
    output.RestartStrip();
    
    p0.coordinate = float3(0.f, 0.f, 1.f);
    output.Append(p0);
    
    PSInput p3;
    float3 N = input[0].normal;
    float3 T = input[0].tangent;
    float3 v3 = cross(N, T);
    
    p3.worldPoition = p0.worldPoition + 0.02f* v3;
    position = float4(p3.worldPoition, 1.f);
    position = mul(position, View);
    position = mul(position, Projection);
    p3.svPosition = position;
    p3.coordinate = float3(1.f, 0.f, 1.f);
    output.Append(p3);
    
    output.RestartStrip();
   
}