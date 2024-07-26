cbuffer cbPerObject : register(b0)
{
    matrix Model;
}

cbuffer cbPass : register(b1)
{
    matrix View;
    matrix Projection;
}
float4 main( float3 pos : POSITION ) : SV_POSITION
{
    float4 position;

    position = mul(float4(pos, 1.f), Model);
    position = mul(position, View);
    position = mul(position, Projection);

    return position;
}