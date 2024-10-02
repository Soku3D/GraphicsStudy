#include "Utility.hlsli"
#include "Smoke.hlsli"

RWTexture3D<float4> velocity : register(u0);
RWTexture3D<float> density : register(u1);

Texture3D<float4> velocityUp : register(t0);
Texture3D<float> densityUp : register(t1);


[numthreads(16, 16, 4)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    float4 velocitySum = float4(0, 0, 0, 0);
    float densitySum = 0.0;
    
    uint upScale = gConstantBuffer.upScale;
    uint3 DTidUp = DTid * upScale;
    
    [loop]
    for (int k = 0; k < upScale; k++)
        for (int j = 0; j < upScale; j++)
            for (int i = 0; i < upScale; i++)
            {
                // TODO:
                velocitySum += velocityUp[DTidUp + uint3(i, j, k)];
                densitySum += densityUp[DTidUp + uint3(i, j, k)];
            }
    
    float scale = 1.0 / (upScale * upScale * upScale);
    
    velocity[DTid] = velocitySum * scale;
    density[DTid] = densitySum * scale;
}
