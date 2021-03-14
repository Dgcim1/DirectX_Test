#pragma once

#include "Object3D.h"

/// <summary>
/// Набор векторов (перемещения, вращения, масштаба) и мировая матрица (матрица мировых координат)
/// </summary>
struct SComponentTransform
{
	/// <summary>
	/// Вектор перемещения
	/// </summary>
	XMVECTOR Translation{};
	/// <summary>
	/// Вектор вращения
	/// </summary>
	XMVECTOR Rotation{};
	/// <summary>
	/// Вектор масштаба
	/// </summary>
	XMVECTOR Scaling{ XMVectorSet(1, 1, 1, 0) };
	/// <summary>
	/// Мировая матрица (матрица мировых координат)
	/// </summary>
	XMMATRIX MatrixWorld{ XMMatrixIdentity() };
};

/// <summary>
/// Указатель на 3D-составляющую игрового обьекта для рендеринга
/// </summary>
struct SComponentRender
{
	/// <summary>
	/// Указатель на 3D обьект
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
	/// Обновление матрицы мира (путем перемножения текущих матриц масштаба, вращения и перемещения)
	/// </summary>
	void UpdateWorldMatrix();

public:
	/// <summary>
	/// Набор векторов (перемещения, вращения, масштаба) и мировая матрица (матрица мировых координат) обьекта
	/// </summary>
	SComponentTransform	ComponentTransform{};
	/// <summary>
	/// Указатель на 3D-составляющую игрового обьекта для рендеринга
	/// </summary>
	SComponentRender	ComponentRender{};
};