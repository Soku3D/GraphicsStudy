#include "RenderBoundingBoxPass.hlsli"



[maxvertexcount(20)]
void main(
	point GSInput input[1],
	inout LineStream<PSInput> output
)
{
	PSInput element;
    float x = boundingBoxHalfLengthX;
    float y = boundingBoxHalfLengthY;
    float z = boundingBoxHalfLengthZ;
    if (x > 0.f && y > 0.f && z > 0.f)
    {
        float4 p0 = float4(-x, -y, -z, 1);
        float4 p1 = float4(-x, -y, z, 1);
        float4 p2 = float4(x, -y, z, 1);
        float4 p3 = float4(x, -y, -z, 1);
        float4 p4 = float4(-x, y, -z, 1);
        float4 p5 = float4(-x, y, z, 1);
        float4 p6 = float4(x, y, z, 1);
        float4 p7 = float4(x, y, -z, 1);
  
        p0 = ApplyMVPMatrix(p0);
        p1 = ApplyMVPMatrix(p1);
        p2 = ApplyMVPMatrix(p2);
        p3 = ApplyMVPMatrix(p3);
        p4 = ApplyMVPMatrix(p4);
        p5 = ApplyMVPMatrix(p5);
        p6 = ApplyMVPMatrix(p6);
        p7 = ApplyMVPMatrix(p7);
    
        element.svPosition = p0;
        output.Append(element);
        element.svPosition = p1;
        output.Append(element);
        element.svPosition = p2;
        output.Append(element);
        element.svPosition = p3;
        output.Append(element);
        element.svPosition = p0;
        output.Append(element);
    
        element.svPosition = p4;
        output.Append(element);
        element.svPosition = p5;
        output.Append(element);
        element.svPosition = p6;
        output.Append(element);
        element.svPosition = p7;
        output.Append(element);
        element.svPosition = p4;
        output.Append(element);
    
        output.RestartStrip();
    
        element.svPosition = p1;
        output.Append(element);
        element.svPosition = p5;
        output.Append(element);
        output.RestartStrip();
	
        element.svPosition = p2;
        output.Append(element);
        element.svPosition = p6;
        output.Append(element);
        output.RestartStrip();
	
        element.svPosition = p3;
        output.Append(element);
        element.svPosition = p7;
        output.Append(element);
        output.RestartStrip();
    }
   
}