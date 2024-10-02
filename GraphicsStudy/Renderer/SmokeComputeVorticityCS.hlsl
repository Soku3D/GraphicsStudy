#include "Utility.hlsli"
#include "Smoke.hlsli"

RWTexture3D<float4> vorticity : register(u0);
Texture3D<float4> velocityUp : register(t0);
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
static int3 offset2[3] =
{
    int3(1, 0, 0),
    int3(0, 1, 0),
    int3(0, 0, 1)
};
[numthreads(16, 16, 4)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    if (gBoundaryCondition[DTid.xyz] >= 0)
    {
        float w, h, d;
        velocityUp.GetDimensions(w, h, d);
        uint x = DTid.x;
        uint y = DTid.y;
        uint z = DTid.z;
    
        float dx = 2.f;
        float dy = 2.f;
        float dz = 2.f;
    
        float v[12];
        int vIdx = 0;
        for (uint i = 0; i < 3; ++i)
        {
            for (uint j = 0; j < 2; ++j)
            {
                int idx = i * 2 + j;
                uint3 index = DTid.xyz + offset[idx];
                for (uint k = 0; k < 3; ++k)
                {
                    if (k != i)
                    {
                        v[vIdx++] = dot(velocityUp[index].xyz, offset2[k]);
                    }
                }
            }
        }
        // dUz/dy - dUy/dz
        float vorticityX = (v[7] - v[5]) / dy - (v[11] - v[9]) / dz;
        
        // - dUz/dx + dUx/dz
        float vorticityY = -(v[3] - v[1]) / dx + (v[10] - v[8]) / dz;
        
        // dUy/dx - dUx/dy
        float vorticityZ = (v[2] - v[0]) / dx - (v[6] - v[4]) / dy;
        vorticity[DTid].xyz = float3(vorticityX, vorticityY, vorticityZ);
    }
}