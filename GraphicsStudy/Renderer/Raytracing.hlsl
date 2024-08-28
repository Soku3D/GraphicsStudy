#ifndef RAYTRACING_HLSL
#define RAYTRACING_HLSL

#define HLSL
#include "RaytracingHlslCompat.h"

RWTexture2D<float4> output : register(u0);
RaytracingAccelerationStructure gSceneTlas : register(t0);
ConstantBuffer<SceneConstantBuffer> gSceneCB : register(b0);

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
void Hit(inout RayPayload rayPayload : SV_Payload, BuiltInAttribute attribute)
{
    uint instanceId = InstanceID();
    
    float3 lightPos = float3(0.f, 0.2f, -1.f);
    
    float3 N;
    float4 albedo;
    float3 hitPos = HitWorldPosition();
    float3 L = normalize(lightPos - hitPos);
    if (instanceId == 0)
    {
        N = normalize(hitPos - float3(-0.5f, 0, 0));
        float NoL = max(0.f, dot(L, N));
        albedo = float4(1.f, 0.f, 0.f, 1.f) * NoL;

    }
    else
    {
        N = normalize(hitPos - float3(0.5f, 0, 0));
        float NoL = max(0.f, dot(L, N));
        albedo = float4(0.f, 0.f, 1.f, 1.f) * NoL;
        
    }
    Ray reflectionRay = { hitPos, reflect(WorldRayDirection(), N) };
    float4 reflectionColor = TraceRadianceRay(reflectionRay, rayPayload.recursionDepth);
    
    float4 color = albedo;
    if (dot(reflectionColor, float4(1, 1, 1, 1)) != 0)
    {
        color = albedo * 0.2f + reflectionColor * 0.8f;
    }
    //else
    //{
    //    color = albedo;
    //}
    rayPayload.color = float4(color.rgb, 1.f);
    
    
}
[shader("miss")]
void Miss(inout RayPayload payload : SV_Payload)
{
    payload.color = float4(0.0f, 0.0f, 0.0f, 1.f);
}
#endif // RAYTRACING_HLSL