#include "Utility.hlsli"
#include "Smoke.hlsli"

Texture3D<float4> velocity : register(t0);

// boundary conditions
// -1: Dirichlet condition
// -2: Neumann condition
//  0: Full cell
Texture3D<int> bc : register(t1);

RWTexture3D<float> divergence : register(u0);
RWTexture3D<float> pressure : register(u1);
RWTexture3D<float> pressureTemp : register(u2);
static int3 offset[6] =
{
    int3(1, 0, 0), // right
    int3(-1, 0, 0), // left
    int3(0, 1, 0), // up
    int3(0, -1, 0), // down
    int3(0, 0, 1), // back
    int3(0, 0, -1) // front
};

[numthreads(16, 16, 4)]
void main( uint3 DTid : SV_DispatchThreadID )
{
    if (bc[DTid.xyz] >= 0)
    {
        float div = 0.0;

        [unroll]
        for (int i = 0; i < 6; i++)
        {
            if (bc[DTid.xyz + offset[i]] == -1) // Dirichlet
            {
                div += dot(velocity[DTid.xyz].xyz, float3(offset[i]));
            }
            if (bc[DTid.xyz + offset[i]] == -2) // Neumann
            {
                div += dot(2 * velocity[DTid.xyz + offset[i]].xyz - velocity[DTid.xyz].xyz, float3(offset[i]));
            }
            else
            {
                div += dot(velocity[DTid.xyz + offset[i]].xyz, float3(offset[i]));
            }
        }

        divergence[DTid.xyz] = 0.5 * div;
        pressure[DTid.xyz] = 0.0;
        pressureTemp[DTid.xyz] = 0.0;
    }
}