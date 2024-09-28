#include "Utility.hlsli"
#include "Smoke.hlsli"

RWTexture3D<float4> gVelocity : register(u0);
Texture3D<float> gPressure : register(t0);
Texture3D<int> gBoundaryCondition : register(t1);

[numthreads(16, 16, 4)]
void main( uint3 DTid : SV_DispatchThreadID )
{
    float w, h, d;
    gPressure.GetDimensions(w, h, d);
    uint x = DTid.x;
    uint y = DTid.y;
    uint z = DTid.z;
    
    float dx = 2.f;
    float dy = 2.f;
    float dz = 2.f;
    
    uint3 left = uint3((x == 0 ? 0 : x - 1), y, z);
    uint3 right = uint3((x == w - 1 ? w - 1 : x + 1), y, z);
    uint3 bottom = uint3(x, (y == h - 1 ? h - 1 : y + 1), z);
    uint3 top = uint3(x, (y == 0 ? 0 : y - 1), z);
    uint3 back = uint3(x, y, (z == d - 1 ? d - 1 : z - 1));
    uint3 front = uint3(x, y, (z == 0 ? 0 : z - 1));

    float pl = gPressure[left];
    float pr = gPressure[right];
    float pbot = gPressure[bottom];
    float pt = gPressure[top];
    float pb = gPressure[back];
    float pf = gPressure[front];
    float p = gPressure[DTid.xyz];
    
    float3 divergencePressure = float3((pr - pl) / dx, (pt - pbot) / dy, (pf - pb) / dz);
    
    gVelocity[DTid].xyz -= divergencePressure;

}