RWTexture2D<float4> gOutput : register(u0);

static const float deltaTime = 1.f / 300.f;

[numthreads(32, 32, 1)]
void main( int3 gID : SV_GroupID ,uint3 DTid : SV_DispatchThreadID )
{
    float a = 2.2f;
    float invA = 1 / 2.2f;
    float3 color = pow(gOutput[DTid.xy].rgb, a);
    //color -= 3.f * deltaTime;
    if (DTid.x > 500)
    {
        gOutput[DTid.xy] = float4(color, 1.f);
    }
}