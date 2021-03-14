#pragma once

#include "SharedHeader.h"

/// <summary>
/// �������, ��������� �� ������� ������� � �����
/// </summary>
struct SVertex3D
{
	SVertex3D() {}
	SVertex3D(const XMVECTOR& _Position, const XMVECTOR& _Color) : Position{ _Position }, Color{ _Color } {}
	SVertex3D(const XMVECTOR& _Position, const XMVECTOR& _Color, const XMFLOAT2& _TexCoord) :
		Position{ _Position }, Color{ _Color }, TexCoord{ _TexCoord } {}
	SVertex3D(const XMVECTOR& _Position, const XMFLOAT2& _TexCoord) : Position{ _Position }, TexCoord{ _TexCoord } {}

	/// <summary>
	/// ������ �������
	/// </summary>
	XMVECTOR Position{};
	/// <summary>
	/// ������ �����
	/// </summary>
	XMVECTOR Color{};
	/// <summary>
	/// ���������� ���������� �������� ��������
	/// </summary>
	XMFLOAT2 TexCoord{};
};

/// <summary>
/// ���������� � 3D �������, ��������� �� ������ � ���������
/// </summary>
struct SObject3DData
{
	/// <summary>
	/// ����� ������
	/// </summary>
	vector<SVertex3D>	vVertices{};

	/// <summary>
	/// ����� ��������� (3 ����� - ���������� ������ ������� ������)
	/// </summary>
	vector<STriangle>	vTriangles{};
};

class CObject3D
{
public:
	/// <summary>
	/// ����������� 3D �������
	/// </summary>
	/// <param name="PtrDevice">��������� �� ����������</param>
	/// <param name="PtrDeviceContext">��������� �� �������� ����������</param>
	CObject3D(ID3D11Device* PtrDevice, ID3D11DeviceContext* PtrDeviceContext) :
		m_PtrDevice{ PtrDevice }, m_PtrDeviceContext{ PtrDeviceContext }
	{
		assert(m_PtrDevice);
		assert(m_PtrDeviceContext);
	}
	~CObject3D() {}
	
	/// <summary>
	/// ������� ���������� � ��������� ������ ������� 3D ������� � ����������� �� � ���������� 
	/// </summary>
	/// <param name="Object3DData">��������� �� ������ � 3D �������</param>
	void Create(const SObject3DData& Object3DData);
	/// <summary>
	/// ����������� ���������� � ��������� ������ (��������) � ��������� � ������ ��
	/// </summary>
	void Draw();

private:
	/// <summary>
	/// ��������� �� ����������
	/// </summary>
	ID3D11Device*			m_PtrDevice{};
	/// <summary>
	/// ��������� �� �������� ����������
	/// </summary>
	ID3D11DeviceContext*	m_PtrDeviceContext{};

private:
	SObject3DData			m_Object3DData{};
	/// <summary>
	/// ��������� �� ��������� �����
	/// </summary>
	ComPtr<ID3D11Buffer>	m_VertexBuffer{};
	/// <summary>
	/// ��������� �� ������ �������� ����. ������ ��� - ��� ������ (� ������) ���������, ������� ������ �������������� �� ����� ������ ������.
	/// </summary>
	UINT					m_VertexBufferStride{ sizeof(SVertex3D) };
	/// <summary>
	/// ��������� �� ������ �������� ��������. ������ �������� - ��� ���������� ������ ����� ������ ��������� ������ ������ � ������ ���������, ������� ����� ��������������.
	/// </summary>
	UINT					m_VertexBufferOffset{};
	/// <summary>
	/// ��������� �� ��������� ����� (�������� ����� �� ����������� ������)
	/// </summary>
	ComPtr<ID3D11Buffer>	m_IndexBuffer{};
};