#include "GeometryPass.hlsli"

PSOutput main(PSInput input)
{
    PSOutput output;
    
    output.position = input.worldPoition;
    output.normal = float4(input.normal, 1.f);
    output.ambientColor = g_ambient.Sample(g_sampler, input.uv);
    output.specularColor = float4(material.ambient, material.diffuse, material.specular, material.shininess);
    
    return output;
}