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
    float3 center = input[0].position.xyz;
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
    
    float3 prevPosition = float3(0, radius, 0);
    
    float3 currPosition;
    float4 svPos = float4(center, 1);
    svPos = mul(svPos, View);
    svPos = mul(svPos, Projection);
    p0.svPosition = svPos;
    p0.position = center;
      
    for (int i = 0; i < 10; ++i)
    {
        output.Append(p0);
        svPos = float4(center + prevPosition, 1);
        svPos = mul(svPos, View);
        svPos = mul(svPos, Projection);
        p1.svPosition = svPos;
        p1.position = center + prevPosition;
        
        currPosition.xyz = float3(mul(prevPosition.xy, mat), 0.f);
        svPos = float4(center + currPosition, 1);
        svPos = mul(svPos, View);
        svPos = mul(svPos, Projection);
        p2.svPosition = svPos;
        p2.position = center + currPosition;
        output.Append(p2);
       
        output.Append(p1);
        
        prevPosition = currPosition;
        output.RestartStrip();

    }
    

}