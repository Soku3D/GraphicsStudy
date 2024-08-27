RWTexture2D<float4> gOutput : register(u0);

static const float deltaTime = 1.f / 300.f;

[numthreads(32, 32, 1)]
void main( int3 gID : SV_GroupID ,uint3 DTid : SV_DispatchThreadID )
{
    float4 color = gOutput[DTid.xy];
    color -= 3.f * deltaTime;
    gOutput[DTid.xy] = color;
}