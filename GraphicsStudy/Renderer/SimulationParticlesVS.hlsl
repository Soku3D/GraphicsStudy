#include "SimulationParticles.hlsli"

GSInput main(VSInput input)
{
    GSInput output;
    output.position = particles[input.vId].position;
    output.color = particles[input.vId].color;
    output.radius = particles[input.vId].radius;
    return output;
}