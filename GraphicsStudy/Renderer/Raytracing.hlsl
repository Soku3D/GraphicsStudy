struct SceneConstantBuffer
{
    float4x4 view;
    float3 cameraPos;
    float dummy;
};

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
       
    float4 world = mul(float4(NDC, 0, 1), gSceneCB.view);
    world.xyz /= world.w;
    ray.Origin = gSceneCB.cameraPos;
    
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
    float3 barycentrics = float3(1 - attribute.BaryX - attribute.BaryY, attribute.BaryX, attribute.BaryY);
    payload.color = float4(barycentrics, 1);
}
[shader("miss")]
void Miss(inout Payload payload : SV_Payload)
{
    payload.color = 0.f;
}
