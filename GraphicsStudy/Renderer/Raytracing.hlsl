#ifndef RAYTRACING_HLSL
#define RAYTRACING_HLSL

#include "RaytracingShaderHelper.hlsli"

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
    rayDesc.TMax = 1000.f;
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
    uint instanceId = InstanceID();
    if(instanceId == 0)
    {
        rayPayload.color = g_specularCube.SampleLevel(g_s0, HitWorldPosition(), 0.f);
    }
    else
    {
          //uint16_t
        uint indexSizeInBytes = 2;
        uint indicesPerTrianlge = 3;
        uint indexStride = indexSizeInBytes * indicesPerTrianlge;
        uint offsetBytes = PrimitiveIndex() * indexStride;
    
        uint3 indices = LoadIndices(offsetBytes);
   
        float u = att.BaryX;
        float v = att.BaryY;
        float w = 1 - u - v;
   
        float2 uv = (w * (g_vertices[indices.x].texcoord) + u * (g_vertices[indices.y].texcoord) + v * (g_vertices[indices.z].texcoord)).rg;
        float3 normal = w * (g_vertices[indices.x].normal) + u * (g_vertices[indices.y].normal) + v * (g_vertices[indices.z].normal);
       
        Ray ray;
        ray.direction = normalize(reflect(WorldRayDirection(), normal));
        ray.origin = HitWorldPosition();
    
        float4 reflectionColor = TraceRadianceRay(ray, rayPayload.recursionDepth);
        float4 albedo = g_albedo.SampleGrad(g_s0, uv, 0, 0);
        float4 color = albedo;
    
        if (dot(reflectionColor, float4(1, 1, 1, 1)) != 0)
        {
            color = albedo * 0.2f + reflectionColor * 0.8f;
        }
  
        rayPayload.color = float4(color.rgb, 1.f);
    }
  
}

[shader("miss")]
void Miss(inout RayPayload payload : SV_Payload)
{
    payload.color = float4(0.0f, 0.0f, 0.0f, 1.f);
}
#endif // RAYTRACING_HLSL