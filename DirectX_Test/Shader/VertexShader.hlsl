#include "Header.hlsli"

VS_OUTPUT main(VS_INPUT input)
{
	VS_OUTPUT output;
	output.Position = input.Position;
	//output.Position = float4(-input.Position[0]*2.0f, input.Position[1]*2.0f, input.Position[2], input.Position[3]);
	//output.Position = float4(input.Position[1], input.Position[0], input.Position[2], input.Position[3]);
	output.Color = input.Color;
	return output;
}