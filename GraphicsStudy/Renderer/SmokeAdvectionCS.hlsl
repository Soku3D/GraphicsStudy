#include "Utility.hlsli"
#include "Smoke.hlsli"

RWTexture3D<float> g_density : register(u0);
RWTexture3D<float4> g_velocity : register(u1);

Texture3D<float> g_densityTemp : register(t0);
Texture3D<float4> g_velocityTemp : register(t1);

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
    uint w, h, d;
    g_velocityTemp.GetDimensions(w, h, d);
    float3 dx = float3(1.0 / w, 1.0 / h, 1.0 / d);
    float3 uvw = (DTid.xyz + 0.5) * dx;
    //float3 backPos = DTid.xyz;
    float3 velocity = g_velocityTemp[DTid.xyz].xyz * dx;
    //velocity = float3(1, 0, 0);
    float3 backPos = uvw - velocity * gConstantBuffer.deltaTime;
    
    //g_density[DTid.xyz] = g_densityTemp.SampleLevel(gClampLinearSampler, backPos, 0.f) * 0.99f;
    //g_velocity[DTid.xyz] = g_velocityTemp.SampleLevel(gClampLinearSampler, backPos, 0.f) * 0.99f;
    g_density[DTid.xyz] = g_densityTemp.SampleLevel(gClampLinearSampler, backPos, 0.f);
    g_velocity[DTid.xyz] = g_velocityTemp.SampleLevel(gClampLinearSampler, backPos, 0.f);
    
    //g_density[DTid.xyz] = g_densityTemp[DTid.xyz];
    //g_velocity[DTid.xyz] = g_velocityTemp[DTid.xyz];

}