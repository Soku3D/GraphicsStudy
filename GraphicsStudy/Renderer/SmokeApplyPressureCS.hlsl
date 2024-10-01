#include "Utility.hlsli"
#include "Smoke.hlsli"

RWTexture3D<float4> gVelocity : register(u0);
Texture3D<float> gPressure : register(t0);
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
        gPressure.GetDimensions(w, h, d);
        uint x = DTid.x;
        uint y = DTid.y;
        uint z = DTid.z;
    
        float dx = 2.f;
        float dy = 2.f;
        float dz = 2.f;
    
        float p[6];
    
        for (uint i = 0; i < 6; ++i)
        {
            uint3 currentIndex = DTid.xyz + offset[i];
        
            if (gBoundaryCondition[currentIndex] == 0)
            {
                p[i] = gPressure[currentIndex];
            }
            else if (gBoundaryCondition[currentIndex] == -1)
            {
                p[i] = -gPressure[DTid.xyz];
            }
            else if (gBoundaryCondition[currentIndex] == -2)
            {
                p[i] = gPressure[DTid.xyz];
            }
        }

    
        float3 divergencePressure = float3((p[1] - p[0]) / dx, (p[3] - p[2]) / dy, (p[5] - p[4]) / dz);
    
        gVelocity[DTid] -= float4(divergencePressure, 0.f);
    }
    

}