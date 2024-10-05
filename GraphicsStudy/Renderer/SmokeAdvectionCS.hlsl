#include "Utility.hlsli"
#include "Smoke.hlsli"

Texture3D<float4> velocityTemp : register(t0);
Texture3D<float> densityTemp : register(t1);

RWTexture3D<float4> velocity : register(u0);
RWTexture3D<float> density : register(u1);

float3 GetUVW(float3 screenPosition, uint width, uint height, uint depth)
{
    float x = screenPosition.x / (width - 1);
    float y = screenPosition.y / (height - 1);
    float z = screenPosition.z / (depth - 1);
    
    return float3(x, y, z);
}

[numthreads(16, 16, 4)]
void main(uint3 DTid : SV_DispatchThreadID)
{

    uint width, height, depth;
    velocity.GetDimensions(width, height, depth);
    float3 dx = float3(1.0 / width, 1.0 / height, 1.0 / depth);
    float3 uvw = (DTid.xyz + 0.5) * dx;

    float3 vel = velocityTemp[DTid.xyz].xyz * dx;

    float3 uvwBack = uvw - vel * gConstantBuffer.deltaTime * gConstantBuffer.upScale;

    float coeff = 1.f;
    density[DTid.xyz] = densityTemp.SampleLevel(gClampLinearSampler, uvwBack, 0) * coeff;
    velocity[DTid.xyz] = velocityTemp.SampleLevel(gClampLinearSampler, uvwBack, 0) *coeff;

}