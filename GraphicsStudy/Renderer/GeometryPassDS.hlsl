#include "GeometryPass.hlsli"


[domain("quad")]
PSInput main(
	HS_CONSTANT_DATA_OUTPUT input,
	float2 domain : SV_DomainLocation,
	const OutputPatch<DSInput, NUM_CONTROL_POINTS> patch)
{
    PSInput Output;

    
    float3 v0 = lerp(patch[0].position, patch[2].position, domain.y);
    float3 v1 = lerp(patch[1].position, patch[3].position, domain.y);
    float3 position = lerp(v0, v1, domain.x);
    
    float3 n0 = lerp(patch[0].normal, patch[2].normal, domain.y);
    float3 n1 = lerp(patch[1].normal, patch[3].normal, domain.y);
    float3 normal = lerp(n0, n1, domain.x);
    
    float3 t0 = lerp(patch[0].tangent, patch[2].tangent, domain.y);
    float3 t1 = lerp(patch[1].tangent, patch[3].tangent, domain.y);
    float3 tangent = lerp(t0, t1, domain.x);
    
    float2 uv0 = lerp(patch[0].uv, patch[2].uv, domain.y);
    float2 uv1 = lerp(patch[1].uv, patch[3].uv, domain.y);
    float2 uv = lerp(uv0, uv1, domain.x);
    
    //float2 n00 = normalize(n0.xz);
    //float2 n10 = normalize(n1.xz);
    //float2 n20 = n10 - n00;
    //float3 center = float3(0, position.y, 0);
    //float2 v2 = v1.xz - v0.xz;
    //float r = 0.f;
    //if (n20.x != 0)
    //{
    //    r = v2.x / n20.x;
    //}
    //else if (n20.y != 0)
    //{
    //    r = v2.y / n20.y;
    //}
    //if (r != 0)
    //{
    //    position = center;
    //    position.xz += r * normal.xz;
    //    position = 0.5 * normalize(normal);
    //}
    
    
    if (useHeightMap)
    {
        float height = g_displacement.SampleLevel(g_sampler, uv, 0).r;
        height = height * 2.0 - 1.0;
        
        position.xyz += 0.02f * normal * height;
    }
    //float4 worldPosition = mul(float4(position, 1.f), Model);
    Output.worldPoition = float4(position, 1);
    Output.normal = normal;
    Output.tangent = tangent;
    Output.uv = uv;
    
    float4 svPosition = mul(float4(position, 1.0), View);
    svPosition = mul(svPosition, Projection);
    Output.svPosition = svPosition;
	return Output;
}
