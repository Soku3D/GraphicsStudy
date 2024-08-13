#include "GeometryPass.hlsli"

PSOutput main(PSInput input)
{
    PSOutput output;
    
    output.position = input.worldPoition;
    output.normal = float4(input.normal, 0.f);
    output.diffuseColor = float4(material.diffuse, 1.f);
    output.ambientColor = float4(material.ambient, 1.f) * g_ambient.Sample(g_sampler, input.uv);
    
    return output;
}