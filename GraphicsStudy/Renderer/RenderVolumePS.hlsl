#include "Volume.hlsli"
#define PI 3.141592

float3 GetUVW(float3 posModel)
{
    return (posModel.xyz + 1.0) * 0.5;
}

// https://wallisc.github.io/rendering/2020/05/02/Volumetric-Rendering-Part-2.html
float BeerLambert(float absorptionCoefficient, float distanceTraveled)
{
    return exp(-absorptionCoefficient * distanceTraveled);
}

// Henyey-Greenstein phase function
// Graph: https://www.researchgate.net/figure/Henyey-Greenstein-phase-function-as-a-function-of-O-O-for-isotropic-scattering-g_fig1_338086693
float HenyeyGreensteinPhase(in float3 L, in float3 V, in float aniso)
{
    // V: eye - pos 
    // L: toLight
    // https://www.shadertoy.com/view/7s3SRH
    
    float cosT = dot(L, -V);
    float g = aniso;
    return (1.0 - g * g) / (4.0 * PI * pow(abs(1.0 + g * g - 2.0 * g * cosT), 3.0 / 2.0));
}

// https://github.com/maruel/temperature/blob/master/temperature.go
float3 ToRGB(float kelvin)
{
    if (kelvin == 6500.0f)
    {
        return float3(1.0f, 1.0f, 1.0f);
    }

    float temperature = kelvin * 0.01f;
    if (kelvin < 6500.0f)
    {
        float b = 0.0;
        float r = 1.0;
        float green = temperature - 2.0;
        float g = (-155.25485562709179f - 0.44596950469579133f * green + 104.49216199393888f * log(green));

        if (kelvin > 2000.0f)
        {
            float blue = temperature - 10.0f;
            b = (-254.76935184120902f + 0.8274096064007395f * blue + 115.67994401066147f * log(blue)) * 255.0f;
        }
        return float3(r, g, b);
    }

    float b = 1.0f;
    float red = temperature - 55.0f;
    float r = (351.97690566805693f + 0.114206453784165f * red - 40.25366309332127f * log(red));
    float green = temperature - 50.0f;
    float g = (325.4494125711974f + 0.07943456536662342f * green - 28.0852963507957f * log(green)) * 255.0f;
    return float3(r, g, b);
}

float4 main(PSInput input) : SV_TARGET
{
    float3 eyeModel = eyePosition; // 
    float3 dirModel = normalize(input.worldPoition.xyz - eyeModel);
    
    int numSteps = 128;
    float stepSize = 2.0 / float(numSteps);

    float3 volumeAlbedo = float3(1, 1, 1);
    
    float4 color = float4(0, 0, 0, 1); 
    float3 posModel = input.worldPoition.xyz + dirModel * 1e-6;

    float3 lightDir = float3(0, 1, 0);
    float densityAbsorption = 10.0;
    float3 lightColor = float3(1, 1, 1) * 40.0;
    
    float3 center = float3(0.5f, 0.5f, 0.5f);
    [loop] 
    for (int i = 0; i < numSteps; i++)
    {
        float3 uvw = GetUVW(posModel); 
        float l = length(uvw - center);
        
        float density = gVolumeDensity.SampleLevel(clampLinearSampler, uvw, 0).r;
        // float lighting = lightingTex.SampleLevel(linearClampSampler, uvw, 0).r;
        float lighting = 1.0; 

        if (density > 1e-3)
        {
            float prevAlpha = color.a;
            color.a *= BeerLambert(10.f * density, stepSize);
            float absorptionFromMarch = prevAlpha - color.a;
            
            float phase = HenyeyGreensteinPhase(lightDir, dirModel, 0.3f);
            phase = 1.f;
            color.rgb += absorptionFromMarch * volumeAlbedo * lightColor
                         * density * lighting
                         * phase;
        }
        
        posModel += dirModel * stepSize;
        
        if (abs(posModel.x) > 1 || abs(posModel.y) > 1 || abs(posModel.z) > 1)
            break;
        
        if (color.a < 1e-3)
            break;

    }

    color = saturate(color);
    
    color.a = 1.f - color.a;
    return color;
}
