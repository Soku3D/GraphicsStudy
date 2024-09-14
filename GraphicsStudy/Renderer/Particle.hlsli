struct Particle
{
    float3 position;
    float3 originPosition;
    float3 color;
    float3 prevVelocity;
    float3 currVelocity;
    float3 originVelocity;
    float3 force;
    float pressure;
    float rho;
    float life;
    float radius;
    float mass;
};

#define PI 3.141592
#define SIMULATION_PARTICLE_SIZE 768

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
    //float coeff = 3.0f / (2.0f * 3.141592f);

    //if (q < 1.0f)
    //    return coeff * (2.0f / 3.0f - q * q + 0.5f * q * q * q);
    //else if (q < 2.0f)
    //    return coeff * pow(2.0f - q, 3.0f) / 6.0f;
    //else // q >= 2.0f
    //    return 0.0f;
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
    //float coeff = 3.0f / (2.0f * 3.141592f);

    //if (q < 1.0f)
    //    return coeff * (-2.0f * q + 1.5f * q * q);
    //else if (q < 2.0f)
    //    return coeff * -0.5f * (2.0f - q) * (2.0f - q);
    //else // q >= 2.0f
    //    return 0.0f;
}

float3 Del(float3 xij, float rhoi, float rhoj, float3 Ai, float3 Aj, float h, float m)
{
    float distance = abs(length(xij));
    float q = (distance * 2.f) / h;
    //float3 del = 1.f / (xij);
    
    return (xij / distance) * (rhoi * m * (Ai / pow(rhoi, 2.f) + Aj / pow(rhoj, 2.f)) * DelKernel(q));
}

float3 LaplaceOperator(float3 xij, float rhoi, float rhoj, float3 Ai, float3 Aj, float h, float m)
{
    float3 Aij = Ai - Aj;
    
    float distance = abs(length(xij));
    float q = (distance * 2.f) / h;
    float3 del = 1.f / (xij);
     
    return Aij * 2.f * (m / rhoj) * ((dot(xij, xij / distance) * DelKernel(q)) / (dot(xij, xij) + 0.01 * pow(h, 2.f)));

}

float ComputeWeight(float3 xij, float h)
{
    float distance = length(xij);
    float f = Kernel((distance * 2.f) / h);

    return f;
}

