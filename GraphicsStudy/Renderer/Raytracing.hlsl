RWTexture2D<float4> output : register(u0);
RaytracingAccelerationStructure gSceneTlas : register(t0);

struct Payload
{
    float4 Color;
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
    float2 NDC = ((launchIndex / dimensions) * 2.f) -1.f;
    NDC.y *= -1.f;

    RayDesc ray;
    ray.Origin = float3(NDC, -1.f);
    ray.Direction = float3(0, 0, 1);
    ray.TMin = 0.1f;
    ray.TMax = 100.f;
    Payload payload;
    TraceRay(gSceneTlas, RAY_FLAG_NONE, ~0, 0, 1, 0, ray, payload);

    output[launchIndex] = payload.Color;

}

[shader("closesthit")]
void Hit(inout Payload payload : SV_Payload, BuiltInAttribute attribute)
{
    payload.Color = 1.f;
}
[shader("miss")]
void Miss(inout Payload payload : SV_Payload)
{
    payload.Color = 0.f;
}
