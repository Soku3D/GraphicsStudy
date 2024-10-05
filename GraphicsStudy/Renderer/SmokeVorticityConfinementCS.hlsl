#include "Utility.hlsli"
#include "Smoke.hlsli"

//RWTexture3D<float4> gVelocity : register(u0);
//Texture3D<float> gDensity : register(t0);
//Texture3D<float4> gVorticity : register(t1);
//Texture3D<int> gBoundaryCondition : register(t2);


RWTexture3D<float4> velocityUp : register(u0);

Texture3D<float4> velocityUpTemp : register(t0);
Texture3D<float> densityUpTemp : register(t1);

static int3 offset[6] =
{
    int3(-1, 0, 0),
    int3(1, 0, 0),
    int3(0, -1, 0),
    int3(0, 1, 0),
    int3(0, 0, -1),
    int3(0, 0, 1)
};

//[numthreads(16, 16, 4)]
//void main(uint3 DTid : SV_DispatchThreadID)
//{
 
//    float width, height, depth;
//    gVelocity.GetDimensions(width, height, depth);
//    float density = gDensity[DTid.xyz];
        
//    float dx = 2.f;
//    float dy = 2.f;
//    float dz = 2.f;
//    if (density > 1e-2
//        && DTid.x > 1 && DTid.y > 1 && DTid.z > 1
//        && DTid.x < width - 2 && DTid.y < height - 2 && DTid.z < depth - 2)
//    {
//        float w[6];
//        for (int i = 0; i < 6; i++)
//        {
//            uint3 index = DTid.xyz + offset[i];
//            float wSize = length(gVorticity[index].xyz);
//            w[i] = wSize;
//        }
        
//        float3 eta = float3(w[1] - w[0], w[3] - w[2], w[5] - w[4]) / dx;
//        if (length(eta) > 1e-3)
//        {
//            float3 N = normalize(eta);
//            gVelocity[DTid.xyz].xyz += cross(N, gVorticity[DTid.xyz].xyz) * density * gConstantBuffer.deltaTime 
//                                            * gConstantBuffer.vorticity * depth;
//        }
//    }
        
    
//}

float3 Vorticity(uint3 center)
{
    // velocity = (v1, v2, v3)
    
    // https://www.khanacademy.org/math/multivariable-calculus/multivariable-derivatives/divergence-and-curl-articles/a/curl#:~:text=Curl%20is%20an%20operator%20which%20measures%20rotation%20in,flow%20indicated%20by%20a%20three%20dimensional%20vector%20field.
    float v3_y = velocityUpTemp[center + int3(0, 1, 0)].z - velocityUpTemp[center - int3(0, 1, 0)].z;
    float v2_z = velocityUpTemp[center + int3(0, 0, 1)].y - velocityUpTemp[center - int3(0, 0, 1)].y;
    float v1_z = velocityUpTemp[center + int3(0, 0, 1)].x - velocityUpTemp[center - int3(0, 0, 1)].x;
    float v3_x = velocityUpTemp[center + int3(1, 0, 0)].z - velocityUpTemp[center - int3(1, 0, 0)].z;
    float v2_x = velocityUpTemp[center + int3(1, 0, 0)].y - velocityUpTemp[center - int3(1, 0, 0)].y;
    float v1_y = velocityUpTemp[center + int3(0, 1, 0)].x - velocityUpTemp[center - int3(0, 1, 0)].x;
    
    return float3(v3_y - v2_z, v1_z - v3_x, v2_x - v1_y) * 0.5;
}

[numthreads(16, 16, 4)]
void main(uint3 dtID : SV_DispatchThreadID)
{
    uint width, height, depth;
    velocityUpTemp.GetDimensions(width, height, depth);

    float density = densityUpTemp[dtID.xyz];

    if (density > 1e-2
        && dtID.x > 1 && dtID.y > 1 && dtID.z > 1
        && dtID.x < width - 2 && dtID.y < height - 2 && dtID.z < depth - 2)
    {
        float3 eta;
        eta.x = length(Vorticity(dtID + int3(1, 0, 0))) - length(Vorticity(dtID - int3(1, 0, 0)));
        eta.y = length(Vorticity(dtID + int3(0, 1, 0))) - length(Vorticity(dtID - int3(0, 1, 0)));
        eta.z = length(Vorticity(dtID + int3(0, 0, 1))) - length(Vorticity(dtID - int3(0, 0, 1)));
        
        float l = length(eta);

        if (l > 1e-3)
        {
            float3 N = eta / l;
            velocityUp[dtID.xyz] += float4(cross(N, Vorticity(dtID.xyz)) * gConstantBuffer.deltaTime
                                * density
                                * gConstantBuffer.vorticity
                                * depth,
                                0);
        }
    }
}
