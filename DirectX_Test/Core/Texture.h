#pragma once

#include "SharedHeader.h"

/// <summary>
/// ������������ ����� ��������
/// </summary>
class CTexture
{
	friend class CGameWindow;

public:
	/// <summary>
	/// ����������� ������ ��������
	/// </summary>
	/// <param name="PtrDevice">��������� �� ����������</param>
	/// <param name="PtrDeviceContext">��������� �� �������� ����������</param>
	CTexture(ID3D11Device* PtrDevice, ID3D11DeviceContext* PtrDeviceContext) : 
		m_PtrDevice{ PtrDevice }, m_PtrDeviceContext{ PtrDeviceContext }
	{
		assert(m_PtrDevice);
		assert(m_PtrDeviceContext);
	}
	~CTexture() {}

	/// <summary>
	/// �������� �������� �� �����
	/// </summary>
	/// <param name="TextureFileName">���� � �����</param>
	void CreateFromFile(const wstring& TextureFileName);
	
private:
	/// <summary>
	/// �������� ������� �������� ������� � �������� � ����� ����������� �������
	/// </summary>
	void Use();
private:
	/// <summary>
	/// ��������� �� ����������
	/// </summary>
	ID3D11Device*						m_PtrDevice{};
	/// <summary>
	/// ��������� �� �������� ����������
	/// </summary>
	ID3D11DeviceContext*				m_PtrDeviceContext{};
private:
	/// <summary>
	/// ��������� �� 2D ��������
	/// </summary>
	ComPtr<ID3D11Texture2D>				m_Texture2D{};
	/// <summary>
	/// ��������� �� ��������� ������ (���������, � �������� ������ ����� �������� ������ �� ����� ����������, � ������ ������ ��������)
	/// </summary>
	ComPtr<ID3D11ShaderResourceView>	m_ShaderResourceView{};
};