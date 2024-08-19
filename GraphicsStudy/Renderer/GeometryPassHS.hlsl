#include "GeometryPass.hlsli"

HS_CONSTANT_DATA_OUTPUT CalcHSPatchConstants(
	InputPatch<VSOutput, NUM_CONTROL_POINTS> ip,
	uint PatchID : SV_PrimitiveID)
{
    HS_CONSTANT_DATA_OUTPUT Output;

    if (useTesslation)
    {
        float3 pos = ip[0].position;
        for (int i = 1; i < NUM_CONTROL_POINTS; ++i)
        {
            pos += ip[i].position;
        }
        pos /= NUM_CONTROL_POINTS;
    
        float distance = abs(length(pos - eyePosition));
    
        float edge = customSaturate(MIN_TESSLATION, MAX_TESSLATION, FallOfStart, FallOfEnd, distance);
    
        Output.edges[0] =
        Output.edges[1] =
        Output.edges[2] =
        Output.edges[3] =
        Output.inside[0] =
        Output.inside[1] = edge;
    }
    else
    {
        Output.edges[0] =
        Output.edges[1] =
        Output.edges[2] =
        Output.edges[3] =
        Output.inside[0] =
        Output.inside[1] = 1;
    }

    return Output;
}

[domain("quad")]
[partitioning("integer")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(4)]
[patchconstantfunc("CalcHSPatchConstants")]
[maxtessfactor(64.f)]
DSInput main(
	InputPatch<VSOutput, NUM_CONTROL_POINTS> ip,
	uint i : SV_OutputControlPointID,
	uint PatchID : SV_PrimitiveID)
{
    DSInput Output;

    Output.position = ip[i].position;
    Output.normal = ip[i].normal;
    Output.tangent = ip[i].tangent;
    Output.uv = ip[i].uv;
    return Output;
}
