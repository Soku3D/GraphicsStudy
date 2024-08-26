RWTexture2D<float4> output : register(u0);
RaytracingAccelerationStructure gSceneBlas : register(t0);

[shader("raygeneration")]
void RayGen()
{
    output[DispatchRaysIndex().xy] = float4(0, 1, 0, 1);
}

