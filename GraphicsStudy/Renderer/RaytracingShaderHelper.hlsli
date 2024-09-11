#define HLSL
#include "RaytracingHlslCompat.h"

RWTexture2D<float4> output : register(u0);
RaytracingAccelerationStructure gSceneTlas : register(t0);



ByteAddressBuffer g_indices : register(t1);
StructuredBuffer<RaytracingVertex> g_vertices : register(t2);

Texture2D<float4> g_albedo : register(t3);
Texture2D<float4> g_ao : register(t4);
Texture2D<float4> g_displacement : register(t5);
Texture2D<float4> g_metalness : register(t6);
Texture2D<float4> g_normal : register(t7);
Texture2D<float4> g_roughness : register(t8);

Texture2D<float4> g_brdf : register(t9);
TextureCube<float4> g_irradianceCube : register(t10);
TextureCube<float4> g_albedoCube : register(t11);
TextureCube<float4> g_specularCube : register(t12);



SamplerState g_s0 : register(s0);

ConstantBuffer<SceneConstantBuffer> gSceneCB : register(b0);

ConstantBuffer<PrimitiveConstantBuffer> l_materialCB : register(b1);

uint3 LoadIndices(uint offsetBytes)
{
    //const uint dwordAlignedOffset = offsetBytes & ~3;
    const uint3 four32BitIndices = g_indices.Load3(offsetBytes);
    uint3 indices;
    ////Little Endian
    //if (dwordAlignedOffset == offsetBytes)
    //{
    //    indices.x = four16BitIndices.x & 0xffff;
    //    indices.y = (four16BitIndices.x >> 16) & 0xffff;
    //    indices.z = four16BitIndices.y & 0xffff;
    //}
    //else // Not aligned: { - 0 | 1 2 }
    //{
    //    indices.x = (four16BitIndices.x >> 16) & 0xffff;
    //    indices.y = four16BitIndices.y & 0xffff;
    //    indices.z = (four16BitIndices.y >> 16) & 0xffff;
    //}
    indices.x = four32BitIndices.x;
    indices.y = four32BitIndices.y;
    indices.z = four32BitIndices.z;
    
    return indices;
    
}