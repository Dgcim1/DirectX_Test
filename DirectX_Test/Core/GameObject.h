#pragma once

#include "SharedHeader.h"
#include "Object3D.h"
#include <DirectXCollision.h>

class CObject3D;
class CTexture;

static constexpr float KBoundingSphereRadiusDefault{ 1.0f };

enum class EFlagsGameObjectRendering
{
	None = 0x00,//TODO
	NoCulling = 0x01,
	NoLighting = 0x02,
	NoDepthComparison = 0x04,
};
ENUM_CLASS_FLAG(EFlagsGameObjectRendering)

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
	/// Вектор вращения по квантериону
	/// </summary>
	XMVECTOR	RotationQuaternion{};
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
	/// <summary>
	/// Указатель на текстуру
	/// </summary>
	CTexture* PtrTexture{};
	/// <summary>
	/// Прозрачная ли текстура
	/// </summary>
	bool IsTransparent{ false };
};

struct SBoundingSphere
{
	float		Radius{ KBoundingSphereRadiusDefault };//TODO
	XMVECTOR	CenterOffset{};
};

/// <summary>
/// Физическая компонента обьекта
/// </summary>
struct SComponentPhysics
{
	/// <summary>
	/// Ограничивающая picking сфера
	/// </summary>
	SBoundingSphere	BoundingSphere{};
	/// <summary>
	/// Может ли обьект быть подвергнут picking'у
	/// </summary>
	bool			bIsPickable{ true };
	/// <summary>
	/// Прямоугольник для проверки коллизий (задается через CGameObject::CreateCollision)
	/// </summary>
	BoundingBox		collisionBox;
	/// <summary>
	/// Учитывать ли коллизии с обьектом (задается через CGameObject::CreateCollision)
	/// </summary>
	bool			isCollide{ false };
	XMFLOAT3 Point1;//TODO
	XMFLOAT3 Point2;
};

class CGameObject
{
	friend class CGameWindow;
	friend class CObject3D;
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
	CGameObject(const string& Name) : m_Name{ Name } {}
	~CGameObject() {}
	/// <summary>
	/// Обновление матрицы мира (путем перемножения текущих матриц масштаба, вращения и перемещения)
	/// </summary>
	void UpdateWorldMatrix();
	/// <summary>
	/// Создает квадрат коллизии для модели
	/// </summary>
	void CreateCollision();
private:
	/// <summary>
	/// Имя обьекта
	/// </summary>
	string							m_Name{};

public:
	/// <summary>
	/// Набор векторов (перемещения, вращения, масштаба) и мировая матрица (матрица мировых координат) обьекта
	/// </summary>
	SComponentTransform	ComponentTransform{};
	/// <summary>
	/// Указатель на 3D-составляющую игрового обьекта для рендеринга
	/// </summary>
	SComponentRender	ComponentRender{};
	SComponentPhysics				ComponentPhysics{};//TODO
	EFlagsGameObjectRendering		eFlagsGameObjectRendering{};
};