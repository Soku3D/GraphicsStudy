struct Particle
{
    float3 position;
    float3 originPosition;
    float3 color;
    float3 velocity;
    float3 originVelocity;
    float3 force;
    float pressure;
    float pressureCoeff;
    float viscosity;
    float density;
	float density0;
    float life;
    float radius;
    float mass;
};

#define PI 3.141592
#define SIMULATION_PARTICLE_SIZE 768
#define SPH_SIMULATION_PARTICLE_SIZE SIMULATION_PARTICLE_SIZE * 5

float Kernel(float q)
{
    float coeff = 3.0f / (2.0f * 3.141592f);
    if (q >= 2)
    {
        return 0;
    }
    else if (q > 1)
    {
        return coeff * (pow((2.f - q), 3.f) / 6.f);
    }
    else
    {
        return coeff * ((2.f / 3.f) - pow(q, 2) + (pow(q, 3) * 0.5f));
    }
}

float DelKernel(float q)
{
    float coeff = 3.0f / (2.0f * 3.141592f);
    if (q >= 2)
    {
        return 0;
    }
    else if (q > 1)
    {
        return coeff * -0.5f * pow(2.f - q, 2.f);
    }
    else
    {
        return coeff * (-2.f * q + (pow(q, 2) * 1.5f));
    }
}

float3 Del(float3 xij, float rhoi, float rhoj, float3 Ai, float3 Aj, float h, float m, float dist)
{
    float q = (dist * 2.f) / h;
    
    return (xij / dist) * (rhoi * m * (Ai / pow(rhoi, 2.f) + Aj / pow(rhoj, 2.f)) * DelKernel(q));
}

float3 LaplaceOperator(float3 xij, float rhoi, float rhoj, float3 Ai, float3 Aj, float h, float m, float dist)
{
    float3 Aij = Ai - Aj;

    float q = (dist * 2.f) / h;
   
    return Aij * 2.f * (m / rhoj) * ((dot(xij, xij / dist) * DelKernel(q)) / (dot(xij, xij) + 0.01 * pow(h, 2.f)));

}

float ComputeWeight(float3 xij, float h)
{
    float distance = length(xij);
    float f = Kernel((distance * 2.f) / h);

    return f;
}

