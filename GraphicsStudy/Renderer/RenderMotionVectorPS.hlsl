#include "MotionVector.hlsli"

float2 main(PSInput input) : SV_TARGET
{
     // Divide by w to convert from clip space to normalized device coordinates (NDC)
    float2 currentNDC = input.ClipPos.xy / input.ClipPos.w; // Current frame NDC
    float2 prevNDC = input.PrevClipPos.xy / input.PrevClipPos.w; // Previous frame NDC

    // Compute the motion vector
    float2 motionVector = currentNDC - prevNDC;

    // Normalize and encode motion vector (optional: for display or further use)
    float2 encodedMotion = motionVector * 0.5f + 0.5f;

    // Output the encoded motion vector as RG (you could output the motion vector directly if needed)
    return motionVector;
}