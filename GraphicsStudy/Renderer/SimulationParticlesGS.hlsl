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
    float radius = input[0].radius / 3.f;
    radius = input[0].radius;
    
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
    
    p0.life =
    p1.life =
    p2.life = input[0].life;
    
    float z = clamp(input[0].life, 0.f, 1.f);
    z = 1.f - z;
    float2 prevPosition = float2(0, radius);
    
    float2 currPosition;
    p0.svPosition = float4(center, z, 1);
    p0.position = center;
    
    //float l = radius * sqrt(2);
    //p0.svPosition = float4(center + float2(-l, -l), 0, 1);
    //p1.svPosition = float4(center + float2(-l, l), 0, 1);
    //p2.svPosition = float4(center + float2(l, l), 0, 1);
    //output.Append(p0);
    //output.Append(p1);
    //output.Append(p2);
    //output.RestartStrip();
    
    //p1.svPosition = float4(center + float2(l, -l), 0, 1);
    //output.Append(p0);
    //output.Append(p2);
    //output.Append(p1);
          
    //output.RestartStrip();
    
    for (int i = 0; i < 10; ++i)
    {
        output.Append(p0);
        p1.svPosition = float4(center + prevPosition, z, 1);
        p1.position = center + prevPosition;
        currPosition = mul(prevPosition, mat);
        p2.svPosition = float4(center + currPosition, z, 1);
        p2.position = center + currPosition;
        output.Append(p2);
       
        output.Append(p1);
        
        prevPosition = currPosition;
        output.RestartStrip();

    }
    

}