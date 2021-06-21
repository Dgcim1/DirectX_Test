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

cbuffer cbSpotlights : register(b4)
{
	float4	SpotlightColor_1;
	float3	SpotlightPosition_1;
	float3	SpotlightDirection_1;
	float	SpotlightAngle_1;
	float	SpotlightRange_1;
}

cbuffer cbPointlights1 : register(b5)
{
	float4	PointlightColor_1;
	float4	PointlightPosition_1;
	float	PointlightRange_1;
	float3	Pads_5;
}

cbuffer cbPointlights2 : register(b6)
{
    float4 PointlightColor_2;
    float4 PointlightPosition_2;
    float PointlightRange_2;
    float3 Pads_6;
}

cbuffer cbPointlights3 : register(b7)
{
    float4 PointlightColor_3;
    float4 PointlightPosition_3;
    float PointlightRange_3;
    float3 Pads_7;
}

cbuffer cbPointlights4 : register(b8)
{
    float4 PointlightColor_4;
    float4 PointlightPosition_4;
    float PointlightRange_4;
    float3 Pads_8;
}

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

	// По мере приближения солнца или луны к горизонту интенсивность света уменьшается.
	float Dot = dot(DirectionalLightDirection, float4(0, 1, 0, 0));
	Result.xyz *= pow(Dot, 0.6f);

	return Result;
}

float4 CalculatePoint(float4 PointlightColor, float4 PointlightPosition, float PointlightRange,
	float4 pos, float4 normal, float4 toEye)
{
	// Инициализируем вывод
	float4 ambient = float4(0.0f, 0.0f, 0.0f, 0.1f);
	float4 diffuse = float4(0.0f, 0.0f, 0.0f, 0.01f);
	float4 spec = float4(0.0f, 0.0f, 0.0f, 1.0f);

	// вектор от поверхности к источнику света
	float4 lightVec = PointlightPosition - pos;// pos - world position

	// расстояние от поверхности до источника света
	float d = length(lightVec);

	// Тест светового диапазона
	if (d > PointlightRange)
		return float4(0.0f, 0.0f, 0.0f, 0.0f);

	// Нормализованный световой вектор
	lightVec /= d;

	// Расчет окружающего освещения
	ambient = float4(MaterialAmbient, 1) * PointlightColor;

	// Расчет диффузного отражения и зеркального отражения
	float diffuseFactor = saturate(dot(lightVec, normal));

	// Расширяем, чтобы избежать динамических ветвей
	if(diffuseFactor > 0.0f)
	{
		float4 v = reflect(-lightVec, normal);

		float4 H = normalize(toEye + lightVec);
		float NDotH = saturate(dot(H, normal));
		float SpecularPower = pow(NDotH, SpecularExponent);
		
		diffuse = float4(MaterialDiffuse, 1) * PointlightColor * diffuseFactor; //PhongDiffuse
		spec = float4(MaterialSpecular * PointlightColor.xyz * SpecularPower * SpecularIntensity, 1);
	}

	// угасание света
	float att = (PointlightRange - d) / PointlightRange;
	spec *= att;
	diffuse *= att;
	ambient *= min(AmbientLightIntensity, att);

	return ambient + diffuse + spec;
}

float4 CalculateSpot(float4 SpotlightColor, float4 SpotlightPosition, float SpotlightRange,
	float3 SpotlightDirection, float SpotlightAngle,
	float4 pos, float4 normal, float4 toEye)
{
	// Инициализируем вывод
	float4 ambient = float4(0.0f, 0.0f, 0.0f, 0.1f);
	float4 diffuse = float4(0.0f, 0.0f, 0.0f, 0.01f);
	float4 spec = float4(0.0f, 0.0f, 0.0f, 1.0f);

	// Вектор от поверхности к источнику света
	float4 lightVec = SpotlightPosition - pos; // pos - world position

    // Расстояние от поверхности до источника света
	float d = length(lightVec);

	// Тест дальности
	if (d > SpotlightRange)
		return float4(0.0f, 0.0f, 0.0f, 0.0f);

	// Нормализованный световой вектор
	lightVec /= d;

	// Рассчитываем часть рассеянного света
	ambient = float4(MaterialAmbient, 1) * SpotlightColor;


	// Расчет диффузного отражения и зеркального отражения
	float diffuseFactor = saturate(dot(lightVec, normal));

	// Расширяем, чтобы избежать динамических ветвей
	if (diffuseFactor > 0.0f)
	{
		float4 v = reflect(-lightVec, normal);

		float4 H = normalize(toEye + lightVec);
		float NDotH = saturate(dot(H, normal));
		float SpecularPower = pow(NDotH, SpecularExponent);

		float specFactor = pow(max(dot(v, toEye), 0.0f), SpecularPower);
		
		diffuse = float4(MaterialDiffuse, 1) * SpotlightColor * diffuseFactor; //PhongDiffuse
		spec = float4(MaterialSpecular * SpotlightColor.xyz * SpecularPower * SpecularIntensity, 1);
	}

	// угасание света
	
    float constantAtt = 0.01f;
    float linearAtt = 0.01f;
    float QuadraticAtt = 0.201f;
    float attenuation = 1.0 / (constantAtt + linearAtt * d + QuadraticAtt * d * d);
	
	// cos 45deg = 0.7
	// cos 55deg = 0.57
	// cos 60deg = 0.5
	
	
    float lightCutOffCosin = 0.57f;//внутри 100% свет 
    float lightOuterCutOffCosin = 0.5f;//за углом ничего 
	
    // Находим косинус угла между направлением фонаря и направлением от объекта к свету
    float cosinBetweenLightAndVertex = dot(lightVec, normalize(-float4(SpotlightDirection, 1))); //theta
    float difference = lightCutOffCosin - lightOuterCutOffCosin; //epsilon
    float intensity_angle = clamp((cosinBetweenLightAndVertex - lightOuterCutOffCosin) / difference, 0.0, 1.0); //intensity
	
	
	//float spot = pow(max(dot(-lightVec, float4(SpotlightDirection, 1)), 0.0f), SpotlightAngle);
	float att = (SpotlightRange - d) / SpotlightRange;
	//float att = attenuation;
    spec *= intensity_angle * att;
    diffuse *= intensity_angle * att;
    ambient *= min(AmbientLightIntensity, intensity_angle * att);
	
	return ambient + diffuse + spec;
	//return ambient;
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
		//Result += CalculateDirectional(DiffuseColor, SpecularColor, normalize(EyePosition - input.WorldPosition), normalize(input.WorldNormal));
        //Result += CalculatePoint(
		//	PointlightColor_1, //PointlightColor
		//	PointlightPosition_1, //PointlightPosition
		//	PointlightRange_1, //PointlightRange
		//	input.WorldPosition, //pos
		//	normalize(input.WorldNormal), //normal
		//	normalize(EyePosition - input.WorldPosition) //ToEye
		//	);
        //Result += CalculatePoint(
		//	PointlightColor_2, //PointlightColor
		//	PointlightPosition_2, //PointlightPosition
		//	PointlightRange_2, //PointlightRange
		//	input.WorldPosition, //pos
		//	normalize(input.WorldNormal), //normal
		//	normalize(EyePosition - input.WorldPosition) //ToEye
		//	);
        //Result += CalculatePoint(
		//	PointlightColor_3, //PointlightColor
		//	PointlightPosition_3, //PointlightPosition
		//	PointlightRange_3, //PointlightRange
		//	input.WorldPosition, //pos
		//	normalize(input.WorldNormal), //normal
		//	normalize(EyePosition - input.WorldPosition) //ToEye
		//	);
        //Result += CalculatePoint(
		//	PointlightColor_4, //PointlightColor
		//	PointlightPosition_4, //PointlightPosition
		//	PointlightRange_4, //PointlightRange
		//	input.WorldPosition, //pos
		//	normalize(input.WorldNormal), //normal
		//	normalize(EyePosition - input.WorldPosition) //ToEye
		//	);
		Result += CalculateSpot(
			float4(1.0f, 1.0f, 1.0f, 0.5f), //SpotlightColor
			float4(0.0f, 0.0f, 0.0f, 0.0f), //SpotlightPosition
			18.0f, //SpotlightRange
			float3(0.0f, 0.0f, 1.0f), //SpotlightDirection
			18.0f, //SpotlightAngle
			input.WorldPosition, //pos
			normalize(input.WorldNormal), //normal
			normalize(EyePosition - input.WorldPosition) //ToEye
			);
		//Result += CalculateSpot(
		//	SpotlightColor_1, //SpotlightColor
		//	float4(SpotlightPosition_1, 0.0f), //SpotlightPosition
		//	SpotlightRange_1, //SpotlightRange
		//	SpotlightDirection_1, //SpotlightDirection
		//	SpotlightAngle_1, //SpotlightAngle
		//	input.WorldPosition, //pos
		//	normalize(input.WorldNormal), //normal
		//	normalize(EyePosition - input.WorldPosition) //ToEye
		//	);
	}

	return Result;
}