#include "LightPass.hlsli"
struct PSInput
{
    float4 position : SV_Position;
};
float4 main(PSInput input) : SV_TARGET
{
    float4 color;
    
    for (int i = 0; i < LIGHT_NUM; i++)
    {
        float x = (input.position.x + 1.f)/ 2.f;
        float y = (input.position.y + 1.f) / 2.f;
        y = -y;
        float2 uv = float2(x, y);
        color += ComputeLight(i, uv);
    }
    return color;
}