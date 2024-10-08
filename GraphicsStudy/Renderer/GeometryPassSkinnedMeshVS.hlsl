#include "GeometryPass.hlsli"

struct SkinnedMeshVSInput
{
    float3 position : POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD;
    float3 tangent : TANGENT;
    float4 boneWeights0 : BLENDWEIGHT0;
    float4 boneWeights1 : BLENDWEIGHT1;
    uint4 boneIndices0 : BLENDINDICES0;
    uint4 boneIndices1 : BLENDINDICES1;
};

PSInput main(SkinnedMeshVSInput input)
{
    float weights[8];
    weights[0] = input.boneWeights0.x;
    weights[1] = input.boneWeights0.y;
    weights[2] = input.boneWeights0.z;
    weights[3] = input.boneWeights0.w;
    weights[4] = input.boneWeights1.x;
    weights[5] = input.boneWeights1.y;
    weights[6] = input.boneWeights1.z;
    weights[7] = input.boneWeights1.w;
    
    uint indices[8]; 
    indices[0] = input.boneIndices0.x;
    indices[1] = input.boneIndices0.y;
    indices[2] = input.boneIndices0.z;
    indices[3] = input.boneIndices0.w;
    indices[4] = input.boneIndices1.x;
    indices[5] = input.boneIndices1.y;
    indices[6] = input.boneIndices1.z;
    indices[7] = input.boneIndices1.w;

    float3 posModel = float3(0.0f, 0.0f, 0.0f);
    float3 normalModel = float3(0.0f, 0.0f, 0.0f);
    float3 tangentModel = float3(0.0f, 0.0f, 0.0f);
    
    for (int i = 0; i < 8; ++i)
    {
        int index = indices[i];
        posModel += weights[i] * mul(float4(input.position, 1), boneTransforms[index]).xyz;
        normalModel += weights[i] * mul(input.normal, (float3x3) boneTransforms[index]);
        tangentModel += weights[i] * mul(input.tangent, (float3x3) boneTransforms[index]);
    }

    //input.position = posModel;
    //input.normal = normalModel;
    //input.tangent = tangentModel;
    
    PSInput output;
    float4 pos = float4(input.position, 1.f);
        
    float3 normal = mul(float4(input.normal, 0.f), invTranspose).xyz;
    
    float3 tangent = mul(float4(input.tangent, 0.f), Model).xyz;
    
    if (useHeightMap)
    {
        float height = g_displacement.SampleLevel(g_sampler, input.uv, 0).r;
        height = height * 2.0 - 1.0;
        
        pos.xyz += 0.02f * normal * height;
    }
    output.worldPoition = pos;
     
    pos = mul(pos, Model);
    pos = mul(pos, View);
    pos = mul(pos, Projection);
    
    output.svPosition = pos;
    output.uv = input.uv;
    output.tangent = normalize(tangent);
    output.normal = normalize(normal);

    
    return output;
}