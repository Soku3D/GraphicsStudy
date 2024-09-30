#include "MotionVector.hlsli"

cbuffer GlobalConstantBuffer : register(b0)
{
    matrix View;
    matrix Projection;
    matrix PrevView;
    matrix PrevProj;
    float3 eyePosition;
};
struct Material
{
    float albedo;
    float ao;
    float metallic;
    float roughness;
};

cbuffer ObjectConstantBuffer : register(b1)
{
    matrix Model;
    matrix invTranspose;
    Material material;

    float bUseAoMap;
    float bUseHeightMap;
    float bUseMetalnessMap;
    float bUseNormalMap;
	
    float bUseRoughnessMap;
    float bUseTesslation;
    float boundingBoxHalfLengthX;
    float boundingBoxHalfLengthY;

    float boundingBoxHalfLengthZ;

};

PSInput main(VSInput input)
{
    PSInput output;
    float4 pos = mul(float4(input.position, 1.f), Model);
    
    output.WorldPos = pos.xyz;
     
    pos = mul(pos, View);
    pos = mul(pos, Projection);
    
    float4 prevPos = mul(float4(input.position, 1.f), Model);
    prevPos = mul(prevPos, PrevView);
    prevPos = mul(prevPos, PrevProj);
    
    output.ClipPos = pos;
    output.PrevClipPos = prevPos;
    output.TexCoord = input.uv;
    
    return output;
}