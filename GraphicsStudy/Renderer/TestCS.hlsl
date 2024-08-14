RWTexture2D<float4> gOutput : register(u0); // swapChain

cbuffer g_csConstant : register(b0)
{
    float time;
}

[numthreads(32, 32, 1)]
void main( int3 gID : SV_GroupID ,uint3 DTid : SV_DispatchThreadID )
{
    float expose = 1.f;
    float gamma = 2.2f;
    float invGamma = 1.f / gamma;
    if (DTid.x > 500)
    {
        gOutput[DTid.xy] = float4(pow(gOutput[DTid.xy].rgb, invGamma), gOutput[DTid.xy].a);
        
    }
    else
    {
        gOutput[DTid.xy] = gOutput[DTid.xy];
        
    }
}