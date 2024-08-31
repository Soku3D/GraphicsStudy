#ifndef RAYTRACING_HLSL
#define RAYTRACING_HLSL

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


SamplerState g_s0 : register(s0);

ConstantBuffer<SceneConstantBuffer> gSceneCB : register(b0);

ConstantBuffer<PrimitiveConstantBuffer> l_materialCB : register(b1);
//ConstantBuffer<LocalConstant> g_localCB : register(b0);

struct Ray
{
    float3 origin;
    float3 direction;
};

float3 HitWorldPosition()
{
    return WorldRayOrigin() + RayTCurrent() * WorldRayDirection();
}

struct BuiltInAttribute
{
    float BaryX;
    float BaryY;
};
float4 TraceRadianceRay(in Ray ray, in UINT currentRayRecursionDepth)
{
    if (currentRayRecursionDepth >= MAX_RAY_RECURSION_DEPTH)
    {
        return float4(0, 0, 0, 0);
    }

    // Set the ray's extents.
    RayDesc rayDesc;
    rayDesc.Origin = ray.origin;
    rayDesc.Direction = ray.direction;
    // Set TMin to a zero value to avoid aliasing artifacts along contact areas.
    // Note: make sure to enable face culling so as to avoid surface face fighting.
    rayDesc.TMin = 0;
    rayDesc.TMax = 100;
    RayPayload rayPayload = { float4(0, 0, 0, 0), currentRayRecursionDepth + 1 };
     TraceRay(gSceneTlas, RAY_FLAG_CULL_BACK_FACING_TRIANGLES, ~0,
        0, 1, 0,
        rayDesc,
        rayPayload);
    
    return rayPayload.color;
}

[shader("raygeneration")]
void RayGen()
{
    float2 launchIndex = DispatchRaysIndex().xy + 0.5f;
    float2 dimensions = DispatchRaysDimensions().xy;
    float2 NDC = ((launchIndex / dimensions) * 2.f) - 1.f;
    NDC.y *= -1.f;
           
    float4 world = mul(float4(NDC, 0, 1), gSceneCB.projectionToWorld);
    world.xyz /= world.w;
    
    //RayDesc ray;
       
    //ray.Origin = gSceneCB.cameraPosition.xyz;
    //ray.Direction = normalize(world.xyz - ray.Origin);
    //ray.TMin = 0.1f;
    //ray.TMax = 100.f;
    //RayPayload payload;
    //TraceRay(gSceneTlas, RAY_FLAG_CULL_BACK_FACING_TRIANGLES, ~0,
    //    0, 1, 0,
    //    ray,
    //    payload);

    //output[DispatchRaysIndex().xy] = payload.color;
    Ray ray;
    ray.origin = gSceneCB.cameraPosition.xyz;
    ray.direction = normalize(world.xyz - ray.origin);
   
    UINT currentRecursionDepth = 0;
    float4 color = TraceRadianceRay(ray, currentRecursionDepth);
    output[DispatchRaysIndex().xy] = color;
}

[shader("closesthit")]
void Hit(inout RayPayload rayPayload : SV_Payload, BuiltInAttribute att)
{
    
    //uint16_t

    uint indexSizeInBytes = 2;
    uint indicesPerTrianlge = 3;
    uint indexStride = indexSizeInBytes * indicesPerTrianlge;
    uint offsetBytes = PrimitiveIndex() * indexStride;
    
    const uint dwordAlignedOffset = offsetBytes & ~3;
    const uint2 four16BitIndices = g_indices.Load2(dwordAlignedOffset);
    uint3 indices;
    //Little Endian
    if (dwordAlignedOffset == offsetBytes)
    {
        indices.x = four16BitIndices.x & 0xffff;
        indices.y = (four16BitIndices.x >> 16) & 0xffff;
        indices.z = four16BitIndices.y & 0xffff;
    }
    else // Not aligned: { - 0 | 1 2 }
    {
        indices.x = (four16BitIndices.x >> 16) & 0xffff;
        indices.y = four16BitIndices.y & 0xffff;
        indices.z = (four16BitIndices.y >> 16) & 0xffff;
    }
    
    //////uint instanceId = InstanceID();
    ////rayPayload.color = l_materialCB.color;
    //////float3 lightPos = float3(0.f, 0.2f, -1.f);
    float2 uv0 = float2(0, 1);
    float2 uv1 = float2(0, 0);
    float2 uv2 = float2(1, 0);
    float u = att.BaryX;
    float v = att.BaryY;
    float w = 1 - u - v;
    float2 uv = w * (uv0) + u * (uv1) + v * (uv2);
    float3 position = w * (g_vertices[indices.x].position) + u * (g_vertices[indices.y].position) + v * (g_vertices[indices.z].position);
   
    //float4 red = float4(1, 0, 0, 1);
    //float4 green = float4(0, 1, 0, 1);
    
    //rayPayload.color = clamp(float4(position, 1), 0, 1);
    rayPayload.color = g_normal.SampleGrad(g_s0, uv, 0, 0);

}
[shader("miss")]
void Miss(inout RayPayload payload : SV_Payload)
{
    payload.color = float4(0.0f, 0.0f, 0.0f, 1.f);
}
#endif // RAYTRACING_HLSL