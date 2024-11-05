#include "Cloth.hlsli"

[maxvertexcount(30)]
void main(
	point GSInput input[1],
	inout TriangleStream<PSInput> output)
{
    float4 zero = float4(0, 0, 0, 0);
    
    float4 p0 = float4(input[0].position0.xyz, 1.f);
    float4 p1 = float4(input[0].position1.xyz, 1.f);
    float4 p2 = float4(input[0].position2.xyz, 1.f);
    float4 p3 = float4(input[0].position3.xyz, 1.f);
    
    PSInput p;
    
    float3 p01 = (p1 - p0).xyz;
    float3 p02 = (p2 - p0).xyz;
    float3 p03 = (p3 - p0).xyz;
    
    float3 normal0 = cross(p01, p02);
    float3 normal1 = cross(p02, p03);
    
    
    p0 = mul(p0, View);
    p0 = mul(p0, Projection);
    p1 = mul(p1, View);
    p1 = mul(p1, Projection);
    p2 = mul(p2, View);
    p2 = mul(p2, Projection);
    p3 = mul(p3, View);
    p3 = mul(p3, Projection);
    
    p.normal = normalize(normal0);
    p.svPosition = p0;
    output.Append(p);
    p.svPosition = p1;
    output.Append(p);
    p.svPosition = p2;
    output.Append(p);
    output.RestartStrip();
    
    p.normal = normalize(normal1);
    p.svPosition = p0;
    output.Append(p);
    p.svPosition = p2;
    output.Append(p);
    p.svPosition = p3;
    output.Append(p);
    output.RestartStrip();
    
    

}