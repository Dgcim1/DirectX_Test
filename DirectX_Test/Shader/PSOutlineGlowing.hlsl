#include "Header.hlsli"

SamplerState CurrentSampler : register(s0);
Texture2D CurrentTexture2D : register(t0);

float4 main(VS_OUTPUT input) : SV_TARGET
{
    float4 Result;
    
    float4 TextureTexelColor = CurrentTexture2D.Sample(CurrentSampler, input.UV);
    
    float fTexelX = 1.0 / 1000; //размер текселя по x
    float fTexelY = 1.0 / 600; //размер текселя по y 
    
    float2 sampling1 = input.UV + float2(fTexelX, 0);
    float2 sampling2 = input.UV + float2(-fTexelX, 0);
    float2 sampling3 = input.UV + float2(0, fTexelY);
    float2 sampling4 = input.UV + float2(0, -fTexelY);
    
    float2 intensity_x0 = CurrentTexture2D.Sample(CurrentSampler, sampling1).xy;
    float2 intensity_x1 = CurrentTexture2D.Sample(CurrentSampler, sampling2).xy;
    float2 intensity_diff_x = intensity_x0 - intensity_x1;
    
    float2 intensity_y0 = CurrentTexture2D.Sample(CurrentSampler, sampling3).xy;
    float2 intensity_y1 = CurrentTexture2D.Sample(CurrentSampler, sampling4).xy;
    float2 intensity_diff_y = intensity_y0 - intensity_y1;
    
    float2 maxAbsDifference = max(abs(intensity_diff_x), abs(intensity_diff_y));

    float2 outlines = saturate(maxAbsDifference);
    
    if (outlines.x == 0.0f && outlines.y == 0.0f)
    {
        Result = float4(0.0f, 0.0f, 0.0f, 1.0f);
    }
    else
    {
        Result = float4(1.0f, 1.0f, 1.0f, 0.0f);
    }
    
    //на данном этапе в теории у нас текстура - черная с белым контуром на прозрачном черном фоне
    
    return Result;
}