#include "Utility.hlsli"
#include "Smoke.hlsli"

// High-res
RWTexture3D<float4> velocityUp : register(u0);
RWTexture3D<float> densityUp : register(u1);

// Low-res
Texture3D<float4> velocityOld : register(t0);
Texture3D<float4> velocityNew : register(t1);
Texture3D<float> densityOld : register(t2);
Texture3D<float> densityNew : register(t3);

[numthreads(16, 16, 4)]
void main(uint3 dtID : SV_DispatchThreadID)
{
    float3 uvw = (dtID + 0.5) * gConstantBuffer.dxUp;
    
    float coeff = 1.f; // 0.0: use interpolated from low-res, 1.0: fully diff-upsample
    
    float4 velOld = velocityOld.SampleLevel(gClampLinearSampler, uvw, 0);
    float4 velNew = velocityNew.SampleLevel(gClampLinearSampler, uvw, 0);
    // TODO:
    velocityUp[dtID] = lerp(velNew, velocityUp[dtID] + velNew - velOld, coeff);
    
    float denOld = densityOld.SampleLevel(gClampLinearSampler, uvw, 0);
    float denNew = densityNew.SampleLevel(gClampLinearSampler, uvw, 0);
    // TODO:
    densityUp[dtID] = lerp(denNew, densityUp[dtID] + denNew - denOld, coeff);

}
