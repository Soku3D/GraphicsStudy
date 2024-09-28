#include "Utility.hlsli"
#include "Smoke.hlsli"

RWTexture3D<float> gPressure : register(u0);
Texture3D<float> gPressureTemp : register(t0);
Texture3D<float> gDivergence : register(t1);
Texture3D<int> gBoundaryCondition : register(t2);

[numthreads(16, 16, 4)]
void main( uint3 DTid : SV_DispatchThreadID )
{
    if (DTid.x == 0 && DTid.y == 0 && DTid.z == 0)
    {
        gPressure[DTid.xyz] = 0.f;
        return;
    }
    
    float w, h, d;
    gPressure.GetDimensions(w, h, d);
    uint x = DTid.x;
    uint y = DTid.y;
    uint z = DTid.z;
    
    uint3 left = uint3((x == 0 ? 0 : x - 1), y, z);
    uint3 right = uint3((x == w - 1 ? w - 1 : x + 1), y, z);
    uint3 bottom = uint3(x, (y == h - 1 ? h - 1 : y + 1), z);
    uint3 top = uint3(x, (y == 0 ? 0 : y - 1), z);
    uint3 back = uint3(x, y, (z == d - 1 ? d - 1 : z - 1));
    uint3 front = uint3(x, y, (z == 0 ? 0 : z - 1));

    float pl = gPressureTemp[left];
    float pr = gPressureTemp[right];
    float pbot = gPressureTemp[bottom];
    float pt = gPressureTemp[top];
    float pb = gPressureTemp[back];
    float pf = gPressureTemp[front];

    gPressure[DTid.xyz] = ((pl + pr + pbot + pt + pb + pf) - gDivergence[DTid.xyz]) / 6.f;
}