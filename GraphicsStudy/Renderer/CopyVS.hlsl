struct PSInput
{
    float4 position : SV_Position;
    float2 uv : TEXCOORD;
};

struct VSInput 
{
    float3 position : POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD;
};
PSInput main(VSInput input)
{
    PSInput output;
    output.position = float4(input.position, 1.f);
    output.uv = input.uv;
    
	return output;
}