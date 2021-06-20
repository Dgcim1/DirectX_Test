#include "Header.hlsli"

SamplerState CurrentSampler : register(s0);
Texture2D CurrentTexture2D : register(t0);

cbuffer cbFlags : register(b0)
{
	bool UseTexture;
	bool UseLighting;
	bool2 Pads;
}

cbuffer cbLights : register(b1)
{
	float4	DirectionalLightDirection;
	float4	DirectionalLightColor;
	float3	AmbientLightColor;
	float	AmbientLightIntensity;
}

cbuffer cbMaterial : register(b2)
{
	float3	MaterialAmbient;
	float	SpecularExponent;
	float3	MaterialDiffuse;
	float	SpecularIntensity;
	float3	MaterialSpecular;
	float	Pad;
}

cbuffer cbEye : register(b3)
{
	float4	EyePosition;
}

// cbuffer cbSpotlights : register(b4)
// {
// 	float4	SpotlightColor;
// 	float3	SpotlightPosition;
// 	float3	SpotlightDirection;
// 	float2	SpotlightAngles;
// }
// 
// cbuffer cbPointlights : register(b5)
// {
// 	float4	PointlightColor;
// 	float4	PointlightPosition;
// 	float3	PointlightAtt;
// 	float	PointlightRange;
// }

float4 CalculateAmbient(float4 AmbientColor)
{
	return float4(AmbientColor.xyz * AmbientLightColor * AmbientLightIntensity, 1);
}

float4 CalculateDirectional(float4 DiffuseColor, float4 SpecularColor, float4 ToEye, float4 Normal)
{
	float NDotL = saturate(dot(DirectionalLightDirection, Normal));
	float4 PhongDiffuse = DiffuseColor * DirectionalLightColor * NDotL;

	float4 H = normalize(ToEye + DirectionalLightDirection);
	float NDotH = saturate(dot(H, Normal));
	float SpecularPower = pow(NDotH, SpecularExponent);
	float4 BlinnSpecular = float4(SpecularColor.xyz * DirectionalLightColor.xyz * SpecularPower * SpecularIntensity, 1);

	float4 Result = PhongDiffuse + BlinnSpecular;

	// ѕо мере приближени€ солнца или луны к горизонту интенсивность света уменьшаетс€.
	float Dot = dot(DirectionalLightDirection, float4(0, 1, 0, 0));
	Result.xyz *= pow(Dot, 0.6f);

	return Result;
}

float4 CalculatePoint(float4 PointlightColor, float4 PointlightPosition, float PointlightRange,
	float4 pos, float4 normal, float4 toEye)
{
	// »нициализируем вывод
	float4 ambient = float4(0.0f, 0.0f, 0.0f, 0.1f);
	float4 diffuse = float4(0.0f, 0.0f, 0.0f, 0.01f);
	float4 spec = float4(0.0f, 0.0f, 0.0f, 1.0f);

	// вектор от поверхности к источнику света
	float4 lightVec = PointlightPosition - pos;// pos - world position

	// рассто€ние от поверхности до источника света
	float d = length(lightVec);

	// “ест светового диапазона
	if (d > PointlightRange)
		return float4(0.0f, 0.0f, 0.0f, 0.0f);

	// Ќормализованный световой вектор
	lightVec /= d;

	// –асчет окружающего освещени€
	ambient = float4(MaterialAmbient, 1) * PointlightColor;

	// –асчет диффузного отражени€ и зеркального отражени€
	float diffuseFactor = saturate(dot(lightVec, normal));

	// –асшир€ем, чтобы избежать динамических ветвей
	if(diffuseFactor > 0.0f)
	{
		float4 v = reflect(-lightVec, normal);

		float4 H = normalize(toEye + lightVec);
		float NDotH = saturate(dot(H, normal));
		float SpecularPower = pow(NDotH, SpecularExponent);

		float specFactor = pow(max(dot(v, toEye), 0.0f), SpecularPower);
		
		diffuse = float4(MaterialDiffuse, 1) * PointlightColor * diffuseFactor; //PhongDiffuse
		spec = float4(MaterialSpecular * PointlightColor.xyz * SpecularPower * SpecularIntensity, 1);
	}

	// угасание света
	float att = (PointlightRange - d) / PointlightRange;
	// att = max(att, 0.1f);
	// att = min(att, 1.0f);
	spec *= att;
	diffuse *= att;
	ambient *= AmbientLightIntensity;
	//ambient *= 0;

	return ambient + diffuse + spec;
}

float4 main(VS_OUTPUT input) : SV_TARGET
{
	float4 AmbientColor = float4(MaterialAmbient, 1);
	float4 DiffuseColor = float4(MaterialDiffuse, 1);
	float4 SpecularColor = float4(MaterialSpecular, 1);

	if (UseTexture == true)
	{
		AmbientColor = DiffuseColor = SpecularColor = CurrentTexture2D.Sample(CurrentSampler, input.UV);
	}
	DiffuseColor.xyz *= DiffuseColor.xyz;

	float4 Result = DiffuseColor;
	if (UseLighting == true)
	{
		Result = CalculateAmbient(AmbientColor);
		Result += CalculateDirectional(DiffuseColor, SpecularColor, normalize(EyePosition - input.WorldPosition), normalize(input.WorldNormal));
		Result += CalculatePoint(
			float4(1.0f, 1.0f, 1.0f, 0.5f),//PointlightColor
			float4(0.0f, 0.0f, 0.0f, 1.0f),//PointlightPosition
			10.5f,//PointlightRange
			input.WorldPosition,//pos
			normalize(input.WorldNormal),//normal
			normalize(EyePosition - input.WorldPosition) //ToEye
			);
	}

	return Result;
}