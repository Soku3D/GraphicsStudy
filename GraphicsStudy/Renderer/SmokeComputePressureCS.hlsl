#include "Utility.hlsli"
#include "Smoke.hlsli"

RWTexture3D<float> gPressure : register(u0);
Texture3D<float> gPressureTemp : register(t0);
Texture3D<float> gDivergence : register(t1);
Texture3D<int> gBoundaryCondition : register(t2);

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
void main(uint3 DTid : SV_DispatchThreadID)
{
    if (gBoundaryCondition[DTid.xyz] >= 0)
    {
        float w, h, d;
        gPressure.GetDimensions(w, h, d);
        uint x = DTid.x;
        uint y = DTid.y;
        uint z = DTid.z;
    
        float pressureSum = 0.f;
        
        for (uint i = 0; i < 6; ++i)
        {
            pressureSum += gPressureTemp[DTid.xyz + offset[i]];
        }

        gPressure[DTid.xyz] = (pressureSum - gDivergence[DTid.xyz]) / 6.f;
    }
}