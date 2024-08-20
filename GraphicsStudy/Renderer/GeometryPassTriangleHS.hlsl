#include "GeometryPass.hlsli"

struct HS_CONSTANT_DATA_OUTPUT2
{
	float EdgeTessFactor[3]			: SV_TessFactor;
	float InsideTessFactor			: SV_InsideTessFactor;
};

HS_CONSTANT_DATA_OUTPUT2 CalcHSPatchConstants(
	InputPatch<VSOutput, 3> ip,
	uint PatchID : SV_PrimitiveID)
{
	HS_CONSTANT_DATA_OUTPUT2 Output;

	Output.EdgeTessFactor[0] = 
		Output.EdgeTessFactor[1] = 
		Output.EdgeTessFactor[2] = 
		Output.InsideTessFactor = 1.f;

	return Output;
}

[domain("tri")]
[partitioning("integer")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(3)]
[patchconstantfunc("CalcHSPatchConstants")]
DSInput main(
	InputPatch<VSOutput, 3> ip,
	uint i : SV_OutputControlPointID,
	uint PatchID : SV_PrimitiveID )
{
    DSInput Output;

    Output.position = ip[i].position;
    Output.normal = ip[i].normal;
    Output.tangent = ip[i].tangent;
    Output.uv = ip[i].uv;
	
	return Output;
}
