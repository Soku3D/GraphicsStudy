#ifndef RAYTRACING_HLSL
#define RAYTRACING_HLSL

#define HLSL
#include "RaytracingHlslCompat.h"

RWTexture2D<float4> output : register(u0);
RaytracingAccelerationStructure gSceneTlas : register(t0);
ConstantBuffer<SceneConstantBuffer> gSceneCB : register(b0);

//ConstantBuffer<LocalConstant> g_localCB : register(b0);

struct Payload
{
    float4 color;
};
struct BuiltInAttribute
{
    float BaryX;
    float BaryY;
};
[shader("raygeneration")]
void RayGen()
{
    float2 launchIndex = DispatchRaysIndex().xy + 0.5f;
    float2 dimensions = DispatchRaysDimensions().xy;
    float2 NDC = ((launchIndex / dimensions) * 2.f) - 1.f;
    NDC.y *= -1.f;

    RayDesc ray;
       
    float4 world = mul(float4(NDC, 0, 1), gSceneCB.projectionToWorld);
    world.xyz /= world.w;
    ray.Origin = gSceneCB.cameraPosition.xyz;
    
    ray.Direction = normalize(world.xyz - ray.Origin);
    ray.TMin = 0.1f;
    ray.TMax = 100.f;
    Payload payload;
    TraceRay(gSceneTlas, RAY_FLAG_CULL_BACK_FACING_TRIANGLES, ~0,
        0, 1, 0,
        ray,
        payload);

    output[launchIndex] = payload.color;

}

[shader("closesthit")]
void Hit(inout Payload payload : SV_Payload, BuiltInAttribute attribute)
{
    uint instanceId = InstanceID();
    if (instanceId == 1)
    {
        payload.color = float4(1,1,0, 1);
    }
    else
    {
        float3 barycentrics = float3(1 - attribute.BaryX - attribute.BaryY, attribute.BaryX, attribute.BaryY);
        payload.color = float4(barycentrics, 1);
    }
  
}
[shader("miss")]
void Miss(inout Payload payload : SV_Payload)
{
    payload.color = 0.f;
}
#endif // RAYTRACING_HLSL