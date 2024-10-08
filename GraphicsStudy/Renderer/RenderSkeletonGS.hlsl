#include "RenderSkeleton.hlsli"

#define PI 3.141592

[maxvertexcount(150)]
void main(
	point GSInput input[1],
	inout TriangleStream<PSInput> output
)
{
    // sphere
    int width = 5;
    int height = 5;
    float r1 = 0.01f;
    float r2 = 0.2f;
    
    float theta1 = PI*2 / width;
    float theta2 = PI / height;
    float c1 = cos(theta1);
    float c2 = cos(theta2);
    float s1 = sin(theta1);
    float s2 = sin(theta2);
    float3x3 rotateY = float3x3(c1, 0, s1,
                                 0, 1, 0,
                                -s1, 0, c1);
    
    float3x3 rotateZ = float3x3(c2, s2, 0,
                                -s2, c2, 0,
                                0, 0, 1);
    float3 ptoc = normalize(input[0].parentsPosition - input[0].position);
    PSInput element;
    element.texcoord = float2(1, 0);
    float3 pos = float3(0, r1, 0);
    float3 center = input[0].parentsPosition;
       
    for (int i = 0; i < height; ++i)
    {
        float3 vertex = pos;
        float3 vertex2 = mul(pos, rotateZ);
        
        for (int j = 0; j < width; ++j)
        {
            float3 v0 = vertex;
            vertex = mul(vertex, rotateY);
            float3 v1 = vertex;
            float3 v2 = vertex2;
            vertex2 = mul(vertex2, rotateY);
            float3 v3 = vertex2;
            
            
            v0 += center;
            v1 += center;
            v2 += center;
            v3 += center;
       
            
            float4 svPos0 = mul(float4(v0, 1), View);
            svPos0 = mul(svPos0, Projection);
            
            float4 svPos1 = mul(float4(v1, 1), View);
            svPos1 = mul(svPos1, Projection);
            
            float4 svPos2 = mul(float4(v2, 1), View);
            svPos2 = mul(svPos2, Projection);
            
            float4 svPos3 = mul(float4(v3, 1), View);
            svPos3 = mul(svPos3, Projection);
            
            element.sv_position = svPos2;
            output.Append(element);
            element.sv_position = svPos0;
            output.Append(element);
            element.sv_position = svPos1;
            output.Append(element);
            output.RestartStrip();
            
            element.sv_position = svPos2;
            output.Append(element);
            element.sv_position = svPos1;
            output.Append(element);
            element.sv_position = svPos3;
            output.Append(element);
            output.RestartStrip();
        }
        pos = mul(pos, rotateZ);
    }
        
    //for (int i = 0; i < height; ++i)
    //{
    //    float3 v0 = pos;
    //    float3 v1 = mul(pos, rotateZ);
    //    float3 v2 = center;
    //    v0 += center;
    //    v1 += center;
    //    float4 svPos0 = mul(float4(v0, 1), View);
    //    svPos0 = mul(svPos0, Projection);
    //    float4 svPos1 = mul(float4(v1, 1), View);
    //    svPos1 = mul(svPos1, Projection);
    //    float4 svPos2 = mul(float4(v2, 1), View);
    //    svPos2 = mul(svPos2, Projection);
    //    element.sv_position = svPos0;
    //    output.Append(element);
    //    element.sv_position = svPos2;
    //    output.Append(element);
    //    element.sv_position = svPos1;
    //    output.Append(element);
    //    output.RestartStrip();
        
    //    pos = mul(pos, rotateZ);

    //}
        
 
}