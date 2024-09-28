#include "Utility.hlsli"
#include "Smoke.hlsli"

RWTexture3D<float> gDivergence : register(u0);
Texture3D<float4> gVelocity : register(t0);

[numthreads(16, 16, 4)]
void main( uint3 DTid : SV_DispatchThreadID )
{
    float w, h, d;
    gVelocity.GetDimensions(w, h, d);
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
    uint3 back = uint3(x, y, (z == d - 1 ? d - 1 :  z - 1));
    uint3 front = uint3(x, y, (z == 0 ? 0 : z - 1));

    float4 vl = gVelocity[left];
    float4 vr = gVelocity[right];
    float4 vbot = gVelocity[bottom];
    float4 vt = gVelocity[top];
    float4 vb = gVelocity[back];
    float4 vf = gVelocity[front];

    gDivergence[DTid.xyz] = (vr - vl) / dx + (vt - vbot) / dy + (vf - vb) / dz;
}