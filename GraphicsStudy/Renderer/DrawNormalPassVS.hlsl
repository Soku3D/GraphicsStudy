#include "DrawNormalPass.hlsli"

//GSInput main(VSInput input)
//{
//    GSInput output;
//    float3 pos = mul(float4(input.position, 1.f), Model).xyz;
//    output.worldPoition = pos;
        
//    float3 normal = mul(float4(input.normal, 0.f), Model).xyz;
//    output.normal = normalize(normal);
    
//    float3 tangent = mul(float4(input.tangent, 0.f), Model).xyz;
//    output.tangent = normalize(tangent);
    
//    return output;
//}
PSInput main(VSInput input)
{
    PSInput output;
    float4 pos = mul(float4(input.position, 1.f), Model);
    output.worldPoition = pos;
        
    float4 svPosition = mul(pos, View);
    svPosition = mul(svPosition, Projection);
    output.svPosition = svPosition;
    //output.coordinate = float3(1, 1, 1);
    return output;
}