#include "RenderSkeleton.hlsli"

#define PI 3.141592

[maxvertexcount(128)]
void main(
	point GSInput input[1],
	inout TriangleStream<PSInput> output
)
{
    // sphere
    int width = 7;
    int height = 7;
    float r1 = 0.01f;
    float r2 = 0.2f;
    
    float theta1 = PI * 2 / width;
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
    float3 ptoc = input[0].position - input[0].parentsPosition;
    float ptocL = length(ptoc);
    float3 normPtoc = normalize(ptoc);
    
    
    r1 = ptocL / 10.f;
    PSInput element;
    element.vId = input[0].vId;
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
            element.normal = v2 - center;
            output.Append(element);
            element.sv_position = svPos0;
            element.normal = v0 - center;
            output.Append(element);
            
        }
        pos = mul(pos, rotateZ);
    }
    
    output.RestartStrip();
    if(ptocL>1e-3)
    {
        float3 crossDir = cross(normPtoc, float3(1, 0, 0));
    
        float3 point0 = input[0].parentsPosition + ptoc * 0.25f;
        crossDir = crossDir * r1 * 2.f;
    
        float4 parents_svPos = mul(float4(input[0].parentsPosition, 1.f), View);
        parents_svPos = mul(parents_svPos, Projection);
    
        float4 current_svPos = mul(float4(input[0].position, 1.f), View);
        current_svPos = mul(current_svPos, Projection);
    
        float c = cos(PI * 0.5f);
        float s = sin(PI * 0.5f);
        float x = normPtoc.x;
        float y = normPtoc.y;
        float z = normPtoc.z;
        float3x3 Rn = float3x3(
        c + (1 - c) * x * x, (1 - c) * x * y + s * z, (1 - c) * x * z - s * y,
        (1 - c) * x * y - s * z, c + (1 - c) * y * y, (1 - c) * y * z + s * x,
        (1 - c) * x * z + s * y, (1 - c) * y * z - s * x, c + (1 - c) * z * z);
    
        float3 v = crossDir;
       
           
        for (int i = 0; i < 4; i++)
        {
            float3 v0 = v;
            float3 v1 = mul(v0, Rn);
            
            v0 += point0;
            v1 += point0;
            
            float4 sv0 = mul(float4(v0, 1.f), View);
            sv0 = mul(sv0, Projection);
            float4 sv1 = mul(float4(v1, 1.f), View);
            sv1 = mul(sv1, Projection);
            
            element.normal = normalize(cross(v1 - v0, input[0].parentsPosition - v0));
            element.sv_position = parents_svPos;
            output.Append(element);
            element.sv_position = sv0;
            output.Append(element);
            element.sv_position = sv1;
            output.Append(element);
            output.RestartStrip();
            
            element.normal = normalize(cross(input[0].position - v0, v1 - v0));
            element.sv_position = sv1;
            output.Append(element);
            element.sv_position = sv0;
            output.Append(element);
            element.sv_position = current_svPos;
            output.Append(element);
            output.RestartStrip();
            v = mul(v, Rn);

        }
        
        
     
    }
}
        