struct Particle
{
    float3 position;
    float3 originPosition;
    float3 color;
    float2 velocity;
    float2 originVelocity;
    float3 pressure;
    float rho;
    float life;
    float radius;
};

#define PI 3.141592
#define SIMULATION_PARTICLE_SIZE 768

float Kernel(float q)
{
    if (q >= 2)
    {
        return 0;
    }
    else if (q > 1)
    {
        return pow(2.f - q, 3.f) / 6.f;
    }
    else
    {
        return ((2.f / 3.f) - pow(q, 2) + (pow(q, 3) / 2.f));
    }
}

float DelKernel(float q)
{
    if (q >= 2)
    {
        return 0;
    }
    else if (q > 1)
    {
        return -pow(2.f - q, 2.f) / 2.f;
    }
    else
    {
        return (-2.f * q + (pow(q, 2) * 3.f) / 2.f);
    }
}
float ComputeWeight(float3 Xi, float3 Xj, float h, float hd){
    float distance = abs(length(Xi-Xj));
    float f = (3.f / (2.f*PI)) * Kernel(distance / h);

    return f/hd;
}

