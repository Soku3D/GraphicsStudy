RWTexture2D<float4> gOutput : register(u0);

cbuffer g_csConstant : register(b0)
{
    float time;
}
[numthreads(32, 32, 1)]
void main( int3 gID : SV_GroupID ,uint3 DTid : SV_DispatchThreadID )
{
    float4 yellow = float4(1.f, 1.f, 0.f, 1.f);
    float4 green = float4(0.f, 1.f, 0.f, 1.f);
    if (DTid.x < time*32.f)
    {
        gOutput[DTid.xy] = yellow;
    }
    
}