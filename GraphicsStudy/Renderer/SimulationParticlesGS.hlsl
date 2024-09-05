#include "SimulationParticles.hlsli"

struct GSOutput
{
	float4 pos : SV_POSITION;
};

[maxvertexcount(30)]
void main(
	point GSInput input[1],
	inout TriangleStream<PSInput> output)
{
    float2 center = input[0].position.xy;
    float radius = input[0].radius;
    
    float PIX2 = 3.141592f * 2.f;
    float delTheta = PIX2 / 10.f;
    
    float2x2 mat = float2x2(cos(delTheta), sin(delTheta), 
                            -sin(delTheta), cos(delTheta));
    PSInput p0,p1,p2;
    p0.color =
    p1.color =
    p2.color = input[0].color;
    
    p0.center =
    p1.center =
    p2.center = center;
    
    p0.radius =
    p1.radius =
    p2.radius = radius;
    
    float2 prevPosition = float2(0, radius);
    float2 currPosition;
    p0.svPosition = float4(center, 0, 1);
    p0.position = center;
    
    for (int i = 0; i < 10; ++i)
    {
        output.Append(p0);
        p1.svPosition = float4(center + prevPosition, 0, 1);
        p1.position = center + prevPosition;
        currPosition = mul(prevPosition, mat);
        p2.svPosition = float4(center + currPosition, 0, 1);
        p2.position = center + currPosition;
        output.Append(p2);
       
        output.Append(p1);
        
        prevPosition = currPosition;
        output.RestartStrip();

    }
    

}