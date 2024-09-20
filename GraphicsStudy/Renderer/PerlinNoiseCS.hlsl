RWTexture2D<float4> gTexture : register(u0);
struct SimulationConstant
{
    float delTime;
};
ConstantBuffer<SimulationConstant> gConstantBuffer : register(b0);

float2 n22(float2 p)
{
    float3 a = frac(p.xyx * float3(123.34, 234.34, 345.65));
    a += dot(a, a + 34.45);
    return frac(float2(a.x * a.y, a.y * a.z));
}

float2 get_gradient(float2 pos)
{
    float twoPi = 6.283185;
    float angle = n22(pos).x * twoPi;
    return float2(cos(angle), sin(angle));
}

float perlin_noise(float2 uv, float cells_count)
{
    float2 pos_in_grid = uv * cells_count;
    float2 cell_pos_in_grid = floor(pos_in_grid);
    float2 local_pos_in_cell = (pos_in_grid - cell_pos_in_grid);
    float2 blend = local_pos_in_cell * local_pos_in_cell * (3.0f - 2.0f * local_pos_in_cell);
    
    float2 left_top = cell_pos_in_grid + float2(0, 1);
    float2 right_top = cell_pos_in_grid + float2(1, 1);
    float2 left_bottom = cell_pos_in_grid + float2(0, 0);
    float2 right_bottom = cell_pos_in_grid + float2(1, 0);
    
    float left_top_dot = dot(pos_in_grid - left_top, get_gradient(left_top));
    float right_top_dot = dot(pos_in_grid - right_top, get_gradient(right_top));
    float left_bottom_dot = dot(pos_in_grid - left_bottom, get_gradient(left_bottom));
    float right_bottom_dot = dot(pos_in_grid - right_bottom, get_gradient(right_bottom));
    
    float noise_value = lerp(
                            lerp(left_bottom_dot, right_bottom_dot, blend.x),
                            lerp(left_top_dot, right_top_dot, blend.x),
                            blend.y);
   
    
    return (0.5 + 0.5 * (noise_value / 0.7));
}

float2 GetGradient(float2 intPos, float t)
{
    
    // Uncomment for calculated rand
    float rand = frac(sin(dot(intPos, float2(12.9898, 78.233))) * 43758.5453);;
    
    // Texture-based rand (a bit faster on my GPU)
    //float rand = texture(iChannel0, intPos / 64.0).r;
    
    // Rotate gradient: random starting rotation, random rotation rate
    float angle = 6.283185 * rand + 4.0 * t * rand;
    return float2(cos(angle), sin(angle));
}


float Pseudo3dNoise(float3 pos)
{
    float2 i = floor(pos.xy);
    float2 f = pos.xy - i;
    float2 blend = f * f * (3.0 - 2.0 * f);
    float noiseVal =
        lerp(
            lerp(
                dot(GetGradient(i + float2(0, 0), pos.z), f - float2(0, 0)),
                dot(GetGradient(i + float2(1, 0), pos.z), f - float2(1, 0)),
                blend.x),
            lerp(
                dot(GetGradient(i + float2(0, 1), pos.z), f - float2(0, 1)),
                dot(GetGradient(i + float2(1, 1), pos.z), f - float2(1, 1)),
                blend.x),
        blend.y
    );
    return noiseVal / 0.7; // normalize to about [-1..1]
}



    //// Mouse down: layered noise
    //else
    //{
    //    const int ITERATIONS = 10;
    //    float noiseVal = 0.0;
    //    float sum = 0.0;
    //    float multiplier = 1.0;
    //    for (int i = 0; i < ITERATIONS; i++)
    //    {
    //        vec3 noisePos = vec3(uv, 0.2 * iTime / multiplier);
    //        noiseVal += multiplier * abs(Pseudo3dNoise(noisePos));
    //        sum += multiplier;
    //        multiplier *= 0.6;
    //        uv = 2.0 * uv + 4.3;
    //    }
    //    noiseVal /= sum;
        
    //    // Map to a color palette
    //    fragColor.rgb = 0.5 + 0.5 * cos(6.283185 * (3.0 * noiseVal + vec3(0.15, 0.0, 0.0)));
    //}

[numthreads(32, 32, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
    static float time = 0.f;
    time += 1 / 60.f;
    uint width, height;
    gTexture.GetDimensions(width, height);
    uint x = DTid.x;
    uint y = DTid.y;
    
    float2 uv = (float2(x, y) + 0.5f) / float2(width - 1, height - 1);
    //uv = DTid.xy / height;
 
    
    // Mouse up: show one noise channel
    //if (iMouse.z <= 0.0)
    //{
    //   
    //    
    //}
    
    //float noiseVal = 0.5 + 0.5 * Pseudo3dNoise(float3(uv * 10.0, 1.f));
    float3 noiseVal = float3(0, 0, 0);
    const int ITERATIONS = 10;
    float sum = 0.0;
    float multiplier = 1.0;
    for (int i = 0; i < ITERATIONS; i++)
    {
        float3 noise = perlin_noise(uv, 0.2 * 1.f / multiplier);
        noiseVal += multiplier * abs(noise);
        sum += multiplier;
        multiplier *= 0.6;
        uv = 2.0 * uv + 4.3;
    }
    noiseVal /= sum;
        
        // Map to a color palette
    float3 color = 0.5 + 0.5 * cos(6.283185 * (3.0 * noiseVal + float3(0.15, 0.0, 0.0)));
    float3 col = 0.5 + 0.5 * cos(uv.xyx + float3(0, 2, 4));

    //gTexture[DTid.xy] = float4(perlin_noise(uv, 10.0f), perlin_noise(uv, 10.0f), perlin_noise(uv, 10.0f), 1.f);
    //gTexture[DTid.xy] = float4(noiseVal, noiseVal, noiseVal, 1.f);
    gTexture[DTid.xy] = float4(color, 1.f);
}