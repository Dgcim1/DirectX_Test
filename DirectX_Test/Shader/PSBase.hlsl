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
	float4	SpotlightPosition_1;
	float3	SpotlightDirection_1;
    float	SpotlightCutOffCosin_1; //�� ����� ������
	float3	SpotlightAtt_1;
    float	SpotlightOuterCutOffCosin_1; //������ ���� 100%
}

cbuffer cbPointlights1 : register(b5)
{
	float4	PointlightColor_1;
	float4	PointlightPosition_1;
	float3	PointlightAtt_1;
	float	Pads_5;
}

cbuffer cbPointlights2 : register(b6)
{
    float4	PointlightColor_2;
    float4	PointlightPosition_2;
    float3	PointlightAtt_2;
    float	Pads_6;
}

cbuffer cbPointlights3 : register(b7)
{
    float4	PointlightColor_3;
    float4	PointlightPosition_3;
    float3	PointlightAtt_3;
    float	Pads_7;
}

cbuffer cbPointlights4 : register(b8)
{
    float4	PointlightColor_4;
    float4	PointlightPosition_4;
    float3	PointlightAtt_4;
    float	Pads_8;
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

	// �� ���� ����������� ������ ��� ���� � ��������� ������������� ����� �����������.
	float Dot = dot(DirectionalLightDirection, float4(0, 1, 0, 0));
	Result.xyz *= pow(Dot, 0.6f);

	return Result;
}

float4 CalculatePoint(float4 PointlightColor, float4 PointlightPosition, float3 PointlightAtt,
	float4 pos, float4 normal, float4 toEye)
{
	// �������������� �����
	float4 ambient = float4(0.0f, 0.0f, 0.0f, 0.1f);
	float4 diffuse = float4(0.0f, 0.0f, 0.0f, 0.01f);
	float4 spec = float4(0.0f, 0.0f, 0.0f, 1.0f);

	// ������ �� ����������� � ��������� �����
	float4 lightVec = PointlightPosition - pos;// pos - world position

	// ���������� �� ����������� �� ��������� �����
	float d = length(lightVec);

	// ��������������� �������� ������
	lightVec /= d;

	// ������ ����������� ���������
	ambient = float4(MaterialAmbient, 1) * PointlightColor;

	// ������ ���������� ��������� � ����������� ���������
	float diffuseFactor = saturate(dot(lightVec, normal));

	// ���������, ����� �������� ������������ ������
	if(diffuseFactor > 0.0f)
	{
		float4 v = reflect(-lightVec, normal);

		float4 H = normalize(toEye + lightVec);
		float NDotH = saturate(dot(H, normal));
		float SpecularPower = pow(NDotH, SpecularExponent);
		
		diffuse = float4(MaterialDiffuse, 1) * PointlightColor * diffuseFactor; //PhongDiffuse
		spec = float4(MaterialSpecular * PointlightColor.xyz * SpecularPower * SpecularIntensity, 1);
	}

	// �������� �����
    float att = 1.0 / (PointlightAtt.x + PointlightAtt.y * d + PointlightAtt.z * d * d);
	spec *= att;
	diffuse *= att;
	ambient *= min(AmbientLightIntensity, att);

	return ambient + diffuse + spec;
}

float4 CalculateSpot(float4 SpotlightColor, float4 SpotlightPosition,
	float3 SpotlightDirection, float3 SpotlightAtt,
	float SpotlightCutOffCosin, float SpotlightOuterCutOffCosin,
	float4 pos, float4 normal, float4 toEye)
{
	// �������������� �����
	float4 ambient = float4(0.0f, 0.0f, 0.0f, 0.1f);
	float4 diffuse = float4(0.0f, 0.0f, 0.0f, 0.01f);
	float4 spec = float4(0.0f, 0.0f, 0.0f, 1.0f);

	// ������ �� ����������� � ��������� �����
	float4 lightVec = SpotlightPosition - pos; // pos - world position

    // ���������� �� ����������� �� ��������� �����
	float d = length(lightVec);

	// ��������������� �������� ������
	lightVec /= d;

	// ������������ ����� ����������� �����
	ambient = float4(MaterialAmbient, 1) * SpotlightColor;


	// ������ ���������� ��������� � ����������� ���������
	float diffuseFactor = saturate(dot(lightVec, normal));

	// ���������, ����� �������� ������������ ������
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

	// �������� �����
	
    // ������� ������� ���� ����� ������������ ������ � ������������ �� ������� � �����
    float cosinBetweenLightAndVertex = dot(lightVec, normalize(-float4(SpotlightDirection, 1))); //theta
    float difference = SpotlightCutOffCosin - SpotlightOuterCutOffCosin; //epsilon
    float intensity_angle = clamp((cosinBetweenLightAndVertex - SpotlightOuterCutOffCosin) / difference, 0.0, 1.0); //intensity
    //float intensity_angle = 1.0f; //intensity
	
    float att = 1.0 / (SpotlightAtt.x + SpotlightAtt.y * d + SpotlightAtt.z * d * d);
    //float SpotlightAngle = 20.0f;
    //float SpotlightRange = 30.0f;
    //float spot = pow(max(dot(-lightVec, float4(SpotlightDirection, 1)), 0.0f), SpotlightAngle);
    //float att = (SpotlightRange - d) / SpotlightRange;
    //att = spot / att;
    spec *= intensity_angle * att;
    diffuse *= intensity_angle * att;
    ambient *= min(AmbientLightIntensity, intensity_angle * att);
	
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
			PointlightColor_1, //PointlightColor
			PointlightPosition_1, //PointlightPosition
			PointlightAtt_1, //PointlightAtt
			input.WorldPosition, //pos
			normalize(input.WorldNormal), //normal
			normalize(EyePosition - input.WorldPosition) //ToEye
			);
        Result += CalculatePoint(
			PointlightColor_2, //PointlightColor
			PointlightPosition_2, //PointlightPosition
			PointlightAtt_2, //PointlightAtt
			input.WorldPosition, //pos
			normalize(input.WorldNormal), //normal
			normalize(EyePosition - input.WorldPosition) //ToEye
			);
        Result += CalculatePoint(
			PointlightColor_3, //PointlightColor
			PointlightPosition_3, //PointlightPosition
			PointlightAtt_3, //PointlightAtt
			input.WorldPosition, //pos
			normalize(input.WorldNormal), //normal
			normalize(EyePosition - input.WorldPosition) //ToEye
			);
        Result += CalculatePoint(
			PointlightColor_4, //PointlightColor
			PointlightPosition_4, //PointlightPosition
			PointlightAtt_4, //PointlightAtt
			input.WorldPosition, //pos
			normalize(input.WorldNormal), //normal
			normalize(EyePosition - input.WorldPosition) //ToEye
			);
		
		
		//Result += CalculateSpot(
		//	float4(1.0f, 1.0f, 1.0f, 0.0f), //SpotlightColor
		//	float4(0.0f, 0.0f, 0.0f, 0.0f), //SpotlightPosition
		//	float3(0.0f, 0.0f, 1.0f), //SpotlightDirection
		//	float3(1.0f, 0.14f, 0.07f), //SpotlightAtt
		//	0.57f, //SpotlightCutOffCosin
		//	0.5f, //SpotlightOuterCutOffCosin
		//	input.WorldPosition, //pos
		//	normalize(input.WorldNormal), //normal
		//	normalize(EyePosition - input.WorldPosition) //ToEye
		//	);
		//Result += CalculateSpot(
		//	SpotlightColor_1, //SpotlightColor
		//	SpotlightPosition_1, //SpotlightPosition
		//	SpotlightDirection_1, //SpotlightDirection
		//	float3(1.0f, 0.09f, 0.032f), //SpotlightAtt
		//	SpotlightCutOffCosin_1, //SpotlightCutOffCosin
		//	SpotlightOuterCutOffCosin_1, //SpotlightOuterCutOffCosin
		//	input.WorldPosition, //pos
		//	normalize(input.WorldNormal), //normal
		//	normalize(EyePosition - input.WorldPosition) //ToEye
		//	);
        //Result += CalculateSpot(
		//	SpotlightColor_1, //SpotlightColor
		//	SpotlightPosition_1, //SpotlightPosition
		//	float3(1,0,0), //SpotlightDirection
		//	float3(1.0f, 0.09f, 0.032f), //SpotlightAtt
		//	SpotlightCutOffCosin_1, //SpotlightCutOffCosin
		//	SpotlightOuterCutOffCosin_1, //SpotlightOuterCutOffCosin
		//	input.WorldPosition, //pos
		//	normalize(input.WorldNormal), //normal
		//	normalize(EyePosition - input.WorldPosition) //ToEye
		//	);
        Result += CalculateSpot(
			SpotlightColor_1, //SpotlightColor
			SpotlightPosition_1, //SpotlightPosition
			SpotlightDirection_1, //SpotlightDirection
			SpotlightAtt_1, //SpotlightAtt
			SpotlightCutOffCosin_1, //SpotlightCutOffCosin
			SpotlightOuterCutOffCosin_1, //SpotlightOuterCutOffCosin
			input.WorldPosition, //pos
			normalize(input.WorldNormal), //normal
			normalize(EyePosition - input.WorldPosition) //ToEye
			);
	}

	return Result;
}