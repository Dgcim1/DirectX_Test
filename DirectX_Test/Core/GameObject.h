#pragma once

#include "Object3D.h"

/// <summary>
/// ����� �������� (�����������, ��������, ��������) � ������� ������� (������� ������� ���������)
/// </summary>
struct SComponentTransform
{
	/// <summary>
	/// ������ �����������
	/// </summary>
	XMVECTOR Translation{};
	/// <summary>
	/// ������ ��������
	/// </summary>
	XMVECTOR Rotation{};
	/// <summary>
	/// ������ ��������
	/// </summary>
	XMVECTOR Scaling{ XMVectorSet(1, 1, 1, 0) };
	/// <summary>
	/// ������� ������� (������� ������� ���������)
	/// </summary>
	XMMATRIX MatrixWorld{ XMMatrixIdentity() };
};

/// <summary>
/// ��������� �� 3D-������������ �������� ������� ��� ����������
/// </summary>
struct SComponentRender
{
	/// <summary>
	/// ��������� �� 3D ������
	/// </summary>
	CObject3D* PtrObject3D{};
};

class CGameObject
{
public:
	void* operator new(size_t Size)
	{
		return _aligned_malloc(Size, 16);
	}

	void operator delete(void* Pointer)
	{
		_aligned_free(Pointer);
	}

public:
	CGameObject() {}
	~CGameObject() {}
	/// <summary>
	/// ���������� ������� ���� (����� ������������ ������� ������ ��������, �������� � �����������)
	/// </summary>
	void UpdateWorldMatrix();

public:
	/// <summary>
	/// ����� �������� (�����������, ��������, ��������) � ������� ������� (������� ������� ���������) �������
	/// </summary>
	SComponentTransform	ComponentTransform{};
	/// <summary>
	/// ��������� �� 3D-������������ �������� ������� ��� ����������
	/// </summary>
	SComponentRender	ComponentRender{};
};