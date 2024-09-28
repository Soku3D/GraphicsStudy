#include "Utility.hlsli"
#include "Smoke.hlsli"

RWTexture3D<float> gDivergence : register(u0);
RWTexture3D<float> gPressure : register(u1);
RWTexture3D<float> gPressureTemp : register(u2);
Texture3D<float4> gVelocity : register(t0);
Texture3D<int> gBoundaryCondition : register(t1);

static int3 offset[6] =
{
    int3(-1, 0, 0),
    int3(1, 0, 0),
    int3(0, -1, 0),
    int3(0, 1, 0),
    int3(0, 0, -1),
    int3(0, 0, 1)
};

[numthreads(16, 16, 4)]
void main( uint3 DTid : SV_DispatchThreadID )
{
    if (gBoundaryCondition[DTid.xyz] >= 0)
    {
        float w, h, d;
        gVelocity.GetDimensions(w, h, d);
        uint x = DTid.x;
        uint y = DTid.y;
        uint z = DTid.z;
    
        float dx = 2.f;
    
        float divergence = 0.f;
        for (uint i = 0; i < 6; ++i)
        {
            uint3 idx = DTid.xyz + offset[i];
            
            if (gBoundaryCondition[DTid.xyz] == 0)
            {
                divergence += dot(gVelocity[idx].xyz, float3(offset[i])) * 0.5f;
            }
            else if (gBoundaryCondition[DTid.xyz] == -1)
            {
                divergence += dot(gVelocity[DTid.xyz].xyz, float3(offset[i])) * 0.5f;
            }
            else
            {
                divergence += dot(2.f * gVelocity[idx].xyz - gVelocity[DTid.xyz].xyz, float3(offset[i])) * 0.5f;
            }

        }
        gDivergence[DTid.xyz] = divergence;
        gPressure[DTid.xyz] = 0.f;
        gPressureTemp[DTid.xyz] = 0.f;
    }
}