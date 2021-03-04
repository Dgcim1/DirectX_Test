#include "Header.hlsli"

float4 main(VS_OUTPUT input) : SV_TARGET
{
	return input.Color;
	/*return float4(1.0f, 0.0f, 0.0f, 1.0f);*/
}