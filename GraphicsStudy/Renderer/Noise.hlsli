#ifndef __NOISE_HLSLI__
#define __NOISE_HLSLI__

// https://www.shadertoy.com/view/3dVXDc

// Hash by David_Hoskins
#define UI0 1597334673U
#define UI1 3812015801U
#define UI2 uint2(UI0, UI1)
#define UI3 uint3(UI0, UI1, 2798796415U)
#define UIF (1.0 / float(0xffffffffU))

float3 hash33(float3 p)
{
    uint3 q = uint3(int3(p)) * UI3;
    q = (q.x ^ q.y ^ q.z) * UI3;
    return -1. + 2. * float3(q) * UIF;
}

float remap(float x, float a, float b, float c, float d)
{
    return (((x - a) / (b - a)) * (d - c)) + c;
}

// Gradient noise by iq (modified to be tileable)
float gradientNoise(float3 x, float freq)
{
    // grid
    float3 p = floor(x);
    float3 w = frac(x);
    
    // quintic interpolant
    float3 u = w * w * w * (w * (w * 6. - 15.) + 10.);
    
    // gradients
    float3 ga = hash33(fmod(p + float3(0., 0., 0.), freq));
    float3 gb = hash33(fmod(p + float3(1., 0., 0.), freq));
    float3 gc = hash33(fmod(p + float3(0., 1., 0.), freq));
    float3 gd = hash33(fmod(p + float3(1., 1., 0.), freq));
    float3 ge = hash33(fmod(p + float3(0., 0., 1.), freq));
    float3 gf = hash33(fmod(p + float3(1., 0., 1.), freq));
    float3 gg = hash33(fmod(p + float3(0., 1., 1.), freq));
    float3 gh = hash33(fmod(p + float3(1., 1., 1.), freq));
    
    // projections
    float va = dot(ga, w - float3(0., 0., 0.));
    float vb = dot(gb, w - float3(1., 0., 0.));
    float vc = dot(gc, w - float3(0., 1., 0.));
    float vd = dot(gd, w - float3(1., 1., 0.));
    float ve = dot(ge, w - float3(0., 0., 1.));
    float vf = dot(gf, w - float3(1., 0., 1.));
    float vg = dot(gg, w - float3(0., 1., 1.));
    float vh = dot(gh, w - float3(1., 1., 1.));
	
    // interpolation
    return va +
           u.x * (vb - va) +
           u.y * (vc - va) +
           u.z * (ve - va) +
           u.x * u.y * (va - vb - vc + vd) +
           u.y * u.z * (va - vc - ve + vg) +
           u.z * u.x * (va - vb - ve + vf) +
           u.x * u.y * u.z * (-va + vb + vc - vd + ve - vf - vg + vh);
}

// Tileable 3D worley noise
float worleyNoise(float3 uv, float freq)
{
    float3 id = floor(uv);
    float3 p = frac(uv);
    
    float minDist = 10000.;
    for (float x = -1.; x <= 1.; ++x)
    {
        for (float y = -1.; y <= 1.; ++y)
        {
            for (float z = -1.; z <= 1.; ++z)
            {
                float3 offset = float3(x, y, z);
                float3 h = hash33(fmod(id + offset, float3(freq, freq, freq))) * .5 + .5;
                h += offset;
                float3 d = p - h;
                minDist = min(minDist, dot(d, d));
            }
        }
    }
    
    // inverted worley noise
    return 1. - minDist;
}

// Fbm(Fractional Brownian motion) for Perlin noise based on iq's blog
float perlinfbm(float3 p, float freq, int octaves)
{
    float G = exp2(-.85);
    float amp = 1.;
    float noise = 0.;
    
    [unroll]
    for (int i = 0; i < octaves; ++i)
    {
        noise += amp * gradientNoise(p * freq, freq);
        freq *= 2.;
        amp *= G;
    }
    
    return noise;
}

// Tileable Worley fbm inspired by Andrew Schneider's Real-Time Volumetric Cloudscapes
// chapter in GPU Pro 7.
float worleyFbm(float3 p, float freq)
{
    return worleyNoise(p * freq, freq) * .625 +
        	 worleyNoise(p * freq * 2., freq * 2.) * .25 +
        	 worleyNoise(p * freq * 4., freq * 4.) * .125;
}
float2 GetGradient(float2 intPos, float t)
{
    
    // Uncomment for calculated rand
    float rand = frac(sin(dot(intPos, float2(12.9898, 78.233))) * 43758.5453);;
    
    float angle = 6.283185 * rand + 4.0 * t * rand;
    return float2(sin(angle), cos(angle));
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
    return noiseVal; // normalize to about [-1..1]
}
#endif // __NOISE_HLSLI__