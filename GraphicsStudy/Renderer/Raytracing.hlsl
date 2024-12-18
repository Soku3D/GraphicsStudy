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

RayPayload TraceRadianceRay(in Ray ray, in UINT currentRayRecursionDepth)
{
    if (currentRayRecursionDepth >= MAX_RAY_RECURSION_DEPTH)
    {
        RayPayload payload = { float4(0, 0, 0, 0), MAX_RAY_RECURSION_DEPTH, true };
        return payload;
    }

    // Set the ray's extents.
    RayDesc rayDesc;
    rayDesc.Origin = ray.origin;
    rayDesc.Direction = ray.direction;
    // Set TMin to a zero value to avoid aliasing artifacts along contact areas.
    // Note: make sure to enable face culling so as to avoid surface face fighting.
    rayDesc.TMin = 0;
    rayDesc.TMax = 1000.f;
    RayPayload rayPayload = { float4(0, 0, 0, 0), currentRayRecursionDepth + 1, true };
    
    if (currentRayRecursionDepth == 0)
    {
        // Draw cubeMap
        TraceRay(gSceneTlas, RAY_FLAG_CULL_BACK_FACING_TRIANGLES, 0x01,
        0, 1, 0,
        rayDesc,
        rayPayload);
    }
    else
    {
        // Draw all objects
        TraceRay(gSceneTlas, RAY_FLAG_CULL_BACK_FACING_TRIANGLES, 0xFF /*InstanceMask*/,
        0 /*HitGroupIndex*/,
        1 /*HitGroup*/,
        0 /*MissShader Offset*/,
        rayDesc,
        rayPayload);
    }
   
    
    return rayPayload;
}
bool TraceShadowRayAndReportIfHit(in Ray ray, in UINT currentRayRecursionDepth)
{
    if (currentRayRecursionDepth >= MAX_RAY_RECURSION_DEPTH)
    {
        return false;
    }

    // Set the ray's extents.
    RayDesc rayDesc;
    rayDesc.Origin = ray.origin;
    rayDesc.Direction = ray.direction;
    rayDesc.TMin = 0;
    rayDesc.TMax = 10000;

    ShadowRayPayload shadowPayload = { true };
    TraceRay(gSceneTlas,
        RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH
        | RAY_FLAG_FORCE_OPAQUE // ~skip any hit shaders
        | RAY_FLAG_SKIP_CLOSEST_HIT_SHADER, // ~skip closest hit shaders,
        0x01,
        1,
        1,
        1,
        rayDesc, shadowPayload);

    return shadowPayload.hit;
}

[shader("raygeneration")]
void RayGen()
{
    float2 launchIndex = DispatchRaysIndex().xy + 0.5f;
    float2 dimensions = DispatchRaysDimensions().xy;
    float2 NDC = ((launchIndex / dimensions) * 2.f) - 1.f;
    NDC.y *= -1.f;
           
    float4 world = mul(float4(NDC, 0, 1), gSceneCB.projection);
    float4 cubeWorld = mul(world, gSceneCB.cubeMapView);
    world = mul(world, gSceneCB.view);
    world.xyz /= world.w;
    Ray ray;
    ray.origin = gSceneCB.cameraPosition.xyz;
    ray.direction = normalize(world.xyz - ray.origin);
   
    UINT currentRecursionDepth = 0;
    RayPayload payload = TraceRadianceRay(ray, currentRecursionDepth);
    if (payload.isHit)
    {
        output[DispatchRaysIndex().xy] = payload.color;
    }
    else
    {
        RayDesc rayDesc;
        rayDesc.Origin = float3(0, 0, 0);
        rayDesc.Direction = normalize(cubeWorld.xyz);
        rayDesc.TMin = 0;
        rayDesc.TMax = 1000.f;
        RayPayload rayPayload = { float4(0, 0, 0, 0), 0, true };
    
        TraceRay(gSceneTlas, RAY_FLAG_CULL_BACK_FACING_TRIANGLES, 0x02,
            0, 1, 0,
            rayDesc,
            rayPayload);
        
        output[DispatchRaysIndex().xy] = rayPayload.color;
        
    }
}

[shader("closesthit")]
void Hit(inout RayPayload rayPayload : SV_Payload, BuiltInAttribute att)
{
    uint instanceId = InstanceID();
    if (instanceId == 0 )
    {
        if (rayPayload.recursionDepth == 0)
        {
            rayPayload.color = g_albedoCube.SampleLevel(g_s0, HitWorldPosition(), 0.f);
        }
        else
        {
            rayPayload.color = g_specularCube.SampleLevel(g_s0, HitWorldPosition(), 0.f);
        }
    }
    else
    {
        //uint32_t
        uint indexSizeInBytes = 4;
        uint indicesPerTrianlge = 3;
        uint indexStride = indexSizeInBytes * indicesPerTrianlge;
        uint offsetBytes = PrimitiveIndex() * indexStride;
    
        uint3 indices = LoadIndices(offsetBytes);
   
        float u = att.BaryX;
        float v = att.BaryY;
        float w = 1 - u - v;
   
        float2 uv = (w * (g_vertices[indices.x].texcoord) + u * (g_vertices[indices.y].texcoord) + v * (g_vertices[indices.z].texcoord)).rg;
        float3 normal = w * (g_vertices[indices.x].normal) + u * (g_vertices[indices.y].normal) + v * (g_vertices[indices.z].normal);
        normal = mul(float4(normal, 0), l_materialCB.invTranspose).rgb;
        //rayPayload.color = float4(normal, 1.f);
        
        float3 hitPosition = HitWorldPosition();
        
        Ray shadowRay;
        shadowRay.direction = normalize(float3(-100, 100, 0) - hitPosition);
        shadowRay.origin = hitPosition + shadowRay.direction* 1e-3;
        bool shadowRayHit = TraceShadowRayAndReportIfHit(shadowRay, rayPayload.recursionDepth);
        
        Ray ray;
        ray.direction = normalize(reflect(WorldRayDirection(), normal));
        ray.origin = HitWorldPosition();
    
        RayPayload reflectionPayload = TraceRadianceRay(ray, rayPayload.recursionDepth);
        float4 albedo = g_albedo.SampleGrad(g_s0, uv, 0, 0);
        float4 color = albedo;
    
        if (dot(reflectionPayload.color, float4(1, 1, 1, 1)) != 0)
        {
            color = albedo * 0.8f + reflectionPayload.color * 0.2f;
        }
        rayPayload.color = float4(color.rgb, 1.f);
        if (shadowRayHit)
        {
            rayPayload.color = float4(color.rgb * 0.5f, 1.f);
        }
        else
        {
            rayPayload.color = float4(color.rgb, 1.f);
        }
    }
  
}

[shader("miss")]
void Miss(inout RayPayload payload : SV_Payload)
{
    payload.color = float4(0.0f, 0.0f, 0.0f, 1.f);
    payload.isHit = false;

}

[shader("miss")]
void Miss_ShadwRay(inout ShadowRayPayload payload)
{
    payload.hit = false;
}
#endif // RAYTRACING_HLSL