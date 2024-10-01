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


[shader("raygeneration")]
void RayGen()
{
    float2 launchIndex = DispatchRaysIndex().xy + 0.5f;
    float2 dimensions = DispatchRaysDimensions().xy;
    float2 NDC = ((launchIndex / dimensions) * 2.f) - 1.f;
    NDC.y *= -1.f;
           
    float4 world = mul(float4(NDC, 0, 1), gSceneCB.projection);
    world = mul(world, gSceneCB.view);
    world.xyz /= world.w;

    UINT currentRecursionDepth = 0;
    RayDesc rayDesc;
    rayDesc.Origin = gSceneCB.cameraPosition.xyz;
    rayDesc.Direction = normalize(world.xyz - rayDesc.Origin);
    rayDesc.TMin = 0;
    rayDesc.TMax = 1000.f;
    
    RayPayload rayPayload = { float4(0, 0, 0, 0), 0, true };
    
    TraceRay(gSceneTlas, RAY_FLAG_CULL_BACK_FACING_TRIANGLES, 0xFF,
            0, 1, 0,
            rayDesc,
            rayPayload);
        
    output[DispatchRaysIndex().xy] = rayPayload.color;
        
   
}

[shader("closesthit")]
void Hit(inout RayPayload rayPayload : SV_Payload, BuiltInAttribute att)
{
    uint indexSizeInBytes = 4;
    uint indicesPerTrianlge = 3;
    uint indexStride = indexSizeInBytes * indicesPerTrianlge;
    uint offsetBytes = PrimitiveIndex() * indexStride;
    
    uint3 indices = LoadIndices(offsetBytes);
   
    float u = att.BaryX;
    float v = att.BaryY;
    float w = 1 - u - v;
   
    float2 uv = (w * (g_vertices[indices.x].texcoord) + u * (g_vertices[indices.y].texcoord) + v * (g_vertices[indices.z].texcoord)).rg;
    float c = uv.y;
    c *= 10.f;
    c = floor(c);
    c /= 10.f;
    c += 0.1f;
    rayPayload.color = float4(c, c, c, 1.f);

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