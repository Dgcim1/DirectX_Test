#pragma once

#include <d3dcompiler.h>
#include "SharedHeader.h"

#pragma comment(lib, "d3dcompiler.lib")

/// <summary>
/// ��� �������
/// </summary>
enum class EShaderType
{
	VertexShader,
	PixelShader,
};

/// <summary>
/// ������
/// </summary>
class CShader final
{
public:
	/// <summary>
	/// ����������� �������
	/// </summary>
	/// <param name="PtrDevice">��������� �� ����������</param>
	/// <param name="PtrDeviceContext">��������� �� �������� ����������</param>
	CShader(ID3D11Device* PtrDevice, ID3D11DeviceContext* PtrDeviceContext) :
		m_PtrDevice{ PtrDevice }, m_PtrDeviceContext{ PtrDeviceContext }
	{
		assert(m_PtrDevice); 
		assert(m_PtrDeviceContext); 
	}
	~CShader() {}
	/// <summary>
	/// ����������� � ��������� � ���������� �������� ������
	/// </summary>
	/// <param name="Type">��� �������</param>
	/// <param name="FileName">���� � ����� *.hlsl � ����� �������</param>
	/// <param name="EntryPoint">��� ������� - ����� ����� � ������</param>
	/// <param name="InputElementDescs">��������� �� ������ - ������� ����� ��� ����� IA</param>
	/// <param name="NumElements">���������� ����� ������� ������ � ������� ������� ���������.</param>
	void Create(EShaderType Type, const wstring& FileName, const string& EntryPoint, 
		const D3D11_INPUT_ELEMENT_DESC* InputElementDescs = nullptr, UINT NumElements = 0);

	/// <summary>
	/// ������������� ������ ��� ��������� ����������
	/// </summary>
	void Use();

private:
	/// <summary>
	/// ��������� �� ����������
	/// </summary>
	ID3D11Device*				m_PtrDevice{};
	/// <summary>
	/// ��������� �� �������� ����������
	/// </summary>
	ID3D11DeviceContext*		m_PtrDeviceContext{};

private:
	/// <summary>
	/// ��������� �� ���������������� ��� �������
	/// </summary>
	ComPtr<ID3DBlob>			m_Blob{};
	/// <summary>
	/// ��������� �� ���������������� ���������� ������ (���� m_ShaderType = VertexShader)
	/// </summary>
	ComPtr<ID3D11VertexShader>	m_VertexShader{};
	/// <summary>
	/// ��������� �� ���������������� ���������� ������ (���� m_ShaderType = PixelShader)
	/// </summary>
	ComPtr<ID3D11PixelShader>	m_PixelShader{};
	ComPtr<ID3D11InputLayout>	m_InputLayout{};
	/// <summary>
	/// ��� �������
	/// </summary>
	EShaderType					m_ShaderType{};
};