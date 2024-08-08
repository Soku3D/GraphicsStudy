RWTexture2D<float4> gOutput : register(u0);

[numthreads(256, 1, 1)]
void main( int3 gID : SV_GroupID ,uint3 DTid : SV_DispatchThreadID )
{
    float4 yellow = float4(1.f, 1.f, 0.f, 1.f);
    float4 green = float4(0.f, 1.f, 0.f, 1.f);
    if (gID.x % 2)
    {
        gOutput[DTid.xy] = yellow;
    }
    
}