#include "GeometryPass.hlsli"


struct HS_CONSTANT_DATA_OUTPUT2
{
    float EdgeTessFactor[3] : SV_TessFactor;
    float InsideTessFactor : SV_InsideTessFactor;
};
[domain("tri")]
PSInput main(
	HS_CONSTANT_DATA_OUTPUT2 input,
	float3 domain : SV_DomainLocation,
	const OutputPatch<DSInput, 3> patch)
{
    PSInput Output;
       
    Output.worldPoition = float4(
		patch[0].position * domain.x + patch[1].position * domain.y + patch[2].position * domain.z, 1);
    Output.normal = float3(
		patch[0].normal * domain.x + patch[1].normal * domain.y + patch[2].normal * domain.z);
    Output.tangent = float3(
		patch[0].tangent * domain.x + patch[1].tangent * domain.y + patch[2].tangent * domain.z);
    Output.uv = float2(
		patch[0].uv * domain.x + patch[1].uv * domain.y + patch[2].uv * domain.z);
    
    if (useHeightMap)
    {
        float height =  g_displacement.SampleLevel(g_sampler, Output.uv, 0.f).r;
        height = height * 2.0 - 1.0;
        Output.worldPoition.xyz += Output.normal * height * 0.02f;
    }
    float4 svPosition = mul(Output.worldPoition, View);
    svPosition = mul(svPosition, Projection);
    Output.svPosition = svPosition;
    return Output;
}
