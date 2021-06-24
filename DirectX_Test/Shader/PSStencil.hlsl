#include "Header.hlsli"

SamplerState CurrentSampler : register(s0);
Texture2D CurrentTexture2D : register(t0);

float4 main(VS_OUTPUT input) : SV_TARGET
{
    return float4(1.0f, 1.0f, 1.0f, 0.0f);
}