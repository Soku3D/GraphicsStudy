#include "Cloth.hlsli"

GSInput main(VSInput input)
{
    
    GSInput output;
    int y = input.vId / (int) height;
    int x = input.vId % (int) height;
    output.position0 = particles[input.vId + (int)height].position / 10.f;
    output.position1 = particles[input.vId + (int) height].position / 10.f;
    output.position2 = particles[input.vId + (int) height].position / 10.f;
    output.position3 = particles[input.vId + (int) height].position / 10.f;

    if(y < height-1 && x < width-1)
    {
        output.position0 = particles[input.vId + (int) height].position / 10.f;
        output.position1 = particles[input.vId].position / 10.f;
        output.position2 = particles[input.vId + 1].position / 10.f;
        output.position3 = particles[input.vId + (int) height + 1].position / 10.f;
    }
  
    return output;
}