#pragma once

#include "SharedHeader.h"
#include "Texture.h"

static constexpr uint32_t KMaxWeightCount{ 4 };
static constexpr uint32_t KMaxBoneMatrixCount{ 60 };

class CGameWindow;

/// <summary>
/// Вершина, состоящая из вектора позиции, вектора цвета, вектора текстурных координат и вектора нормали
/// </summary>
struct SVertex3D
{
	SVertex3D() {}
	SVertex3D(const XMVECTOR& _Position, const XMVECTOR& _Color, const XMVECTOR& _TexCoord = XMVectorSet(0, 0, 0, 0)) :
		Position{ _Position }, Color{ _Color }, TexCoord{ _TexCoord } {}

	/// <summary>
	/// Вектор позиции
	/// </summary>
	XMVECTOR Position{};
	/// <summary>
	/// Вектор цвета
	/// </summary>
	XMVECTOR Color{};
	/// <summary>
	/// Координаты начального смещения текстуры
	/// </summary>
	XMVECTOR TexCoord{};
	/// <summary>
	/// Вектор нормали
	/// </summary>
	XMVECTOR Normal{};
};

/// <summary>
/// TODO анимации
/// </summary>
struct SVertexAnimation
{
	uint32_t	BoneIDs[KMaxWeightCount]{};
	float		Weights[KMaxWeightCount]{};
};

/// <summary>
/// Информация о 3D обьекте, состоящая из вершин, полигонов и идентификатора материала
/// </summary>
struct SMesh
{
	/// <summary>
	/// Набор вершин
	/// </summary>
	vector<SVertex3D>	vVertices{};
	/// <summary>
	/// Набор анимируемых вершин
	/// </summary>
	vector<SVertexAnimation>	vVerticesAnimation{};
	/// <summary>
	/// Набор полигонов (3 точки - порядковые номера массива вершин)
	/// </summary>
	vector<STriangle>	vTriangles{};
	/// <summary>
	/// ID материала
	/// </summary>
	size_t				MaterialID{};
};

// TODO
struct SModelNode
{
	struct SBlendWeight
	{
		uint32_t	MeshIndex{};
		uint32_t	VertexID{};
		float		Weight{};
	};

	int32_t					Index{};
	string					Name{};

	int32_t					ParentNodeIndex{};
	vector<int32_t>			vChildNodeIndices{};
	XMMATRIX				MatrixTransformation{};

	bool					bIsBone{ false };
	uint32_t				BoneIndex{};
	vector<SBlendWeight>	vBlendWeights{};
	XMMATRIX				MatrixBoneOffset{};
};

// TODO
struct SModelNodeAnimation
{
	struct SKey
	{
		float		Time{};
		XMVECTOR	Value{};
	};

	uint32_t		Index{};
	string			NodeName{};
	vector<SKey>	vPositionKeys{};
	vector<SKey>	vRotationKeys{};
	vector<SKey>	vScalingKeys{};
};

// TODO
struct SModelAnimation
{
	vector<SModelNodeAnimation>		vNodeAnimations{};

	float							Duration{};
	float							TicksPerSecond{};

	unordered_map<string, size_t>	mapNodeAnimationNameToIndex{};
};

/// <summary>
/// Информация о буферах отдельного меша (модели)
/// </summary>
struct SMeshBuffers
{
	/// <summary>
	/// Указатель на буфер вершин
	/// </summary>
	ComPtr<ID3D11Buffer>	VertexBuffer{};
	/// <summary>
	/// Значение шага. Каждый шаг - это размер (в байтах) элементов, которые должны использоваться из этого буфера вершин.
	/// </summary>
	UINT					VertexBufferStride{ sizeof(SVertex3D) };
	/// <summary>
	/// Значение смещения. Каждое смещение - это количество байтов между первым элементом буфера вершин и первым элементом, который будет использоваться.
	/// </summary>
	UINT					VertexBufferOffset{};

	//TODO то же для анимации
	ComPtr<ID3D11Buffer>	VertexBufferAnimation{};
	UINT					VertexBufferAnimationStride{ sizeof(SVertexAnimation) };
	UINT					VertexBufferAnimationOffset{};

	/// <summary>
	/// Указатель на буфер индексов
	/// </summary>
	ComPtr<ID3D11Buffer>	IndexBuffer{};
};

/// <summary>
/// Представляет собой материал
/// </summary>
struct SMaterial
{
	/// <summary>
	/// Конструктор материала по умолчанию
	/// </summary>
	SMaterial() {}
	/// <summary>
	/// Конструктор материала
	/// </summary>
	/// <param name="UniversalColor">Цвет (материала, света, блика)</param>
	SMaterial(const XMFLOAT3& UniversalColor) : MaterialAmbient{ UniversalColor }, MaterialDiffuse{ UniversalColor }, MaterialSpecular{ UniversalColor } {}
	/// <summary>
	/// Цвет материала (затененной части обьекта)
	/// </summary>
	XMFLOAT3	MaterialAmbient{};
	/// <summary>
	/// Зеркальная экспонента (размер блика, чем больше значение, тем меньше блик и более размытые границы)
	/// </summary>
	float		SpecularExponent{ 1 };
	/// <summary>
	/// Цвет материала (освещенной части материала)
	/// </summary>
	XMFLOAT3	MaterialDiffuse{};
	/// <summary>
	/// Зеркальная интенсивность (насколько интенсивно/ярко светит отраженный от обьекта бликующий свет)
	/// </summary>
	float		SpecularIntensity{ 0 };
	/// <summary>
	/// Цвет материала (цвет блика)
	/// </summary>
	XMFLOAT3	MaterialSpecular{};
	/// <summary>
	/// Имеет ли обьект текстуру
	/// </summary>
	bool		bHasTexture{ false };
	/// <summary>
	/// Имеет ли строенную (загруженную извне, из файла текстуры) текстуру
	/// </summary>
	bool		bHasEmbeddedTexture{ false };
	/// <summary>
	/// Путь к файлу с текстурой
	/// </summary>
	string		TextureFileName{};
	/// <summary>
	/// WIC текстуры (условно говоря сама текстура)
	/// </summary>
	vector<uint8_t>		vEmbeddedTextureRawData{};
};

/// <summary>
/// Представляет собой модель (текстура и точки вершин)
/// </summary>
struct SModel
{
	/// <summary>
	/// Массив мешей (сама модель, ее точки)
	/// </summary>
	vector<SMesh>		vMeshes{};
	/// <summary>
	/// Массив материалов модели (коэффициенты отражения, блик и т п)
	/// </summary>
	vector<SMaterial>	vMaterials{};

	vector<SModelNode>				vNodes{};//TODO
	unordered_map<string, size_t>	mapNodeNameToIndex{};
	uint32_t						ModelBoneCount{};

	vector<SModelAnimation>			vAnimations{};
	bool							bIsAnimated{};
};

/// <summary>
/// Представляет собой 3D обьект
/// </summary>
class CObject3D
{
	friend class CGameWindow;
	friend class CGameObject;
public:
	/// <summary>
	/// Конструктор 3D обьекта
	/// </summary>
	/// <param name="PtrDevice">Указатель на устройство</param>
	/// <param name="PtrDeviceContext">Указатель на контекст устройства</param>
	/// <param name="PtrGameWindow"></param>
	CObject3D(ID3D11Device* PtrDevice, ID3D11DeviceContext* PtrDeviceContext, CGameWindow* PtrGameWindow) :
		m_PtrDevice{ PtrDevice }, m_PtrDeviceContext{ PtrDeviceContext }, m_PtrGameWindow{ PtrGameWindow }
	{
		assert(m_PtrDevice);
		assert(m_PtrDeviceContext);
		assert(m_PtrGameWindow);
	}
	~CObject3D() {}

	/// <summary>
	/// Создает 3D обьект
	/// </summary>
	/// <param name="Mesh">Меш 3D обьекта</param>
	/// <param name="Material">Материал 3D обьекта</param>
	void Create(const SMesh& Mesh, const SMaterial& Material = SMaterial());
	/// <summary>
	/// Создает 3D обьект
	/// </summary>
	/// <param name="vMeshes">Вектор мешей 3D обьекта</param>
	/// <param name="vMaterials">Вектор материалов 3D обьекта</param>
	void Create(const vector<SMesh>& vMeshes, const vector<SMaterial>& vMaterials);
	/// <summary>
	/// Создает 3D обьект из модели
	/// </summary>
	/// <param name="Model">Модель 3D обьекта</param>
	void Create(const SModel& Model);

	void UpdateQuadUV(const XMFLOAT2& UVOffset, const XMFLOAT2& UVSize);

	void Animate();

	std::tuple<XMFLOAT3, XMFLOAT3> GetBoxObject();
private:
	/// <summary>
	/// Создает вертексный и индексный буферы заданного меша данного 3D обьекта и привязывает их к устройству
	/// </summary>
	/// <param name="MeshIndex">Индекс меша в массиве</param>
	/// <param name="IsAnimated">Является ли обьект анимированным</param>
	void CreateMeshBuffers(size_t MeshIndex, bool IsAnimated);
	/// <summary>
	/// Загрузка меша в Vertex Buffer
	/// </summary>
	/// <param name="MeshIndex">Индекс загружаемого меша</param>
	void UpdateMeshBuffer(size_t MeshIndex = 0);
	/// <summary>
	/// TODO animation
	/// </summary>
	/// <param name="Node"></param>
	/// <param name="ParentTransform"></param>
	void CalculateAnimatedBoneMatrices(const SModelNode& Node, XMMATRIX ParentTransform);
	/// <summary>
	/// Привязываем вертексный и индексный буферы (полигоны) к конвейеру и рисуем их
	/// </summary>
	void Draw() const;
	/// <summary>
	/// Отрисовывает нормали
	/// </summary>
	void DrawNormals() const;
private:
	/// <summary>
	/// Указатель на устройство
	/// </summary>
	ID3D11Device*			m_PtrDevice{};
	/// <summary>
	/// Указатель на контекст устройства
	/// </summary>
	ID3D11DeviceContext*	m_PtrDeviceContext{};
	/// <summary>
	/// Указатель на игровое окно
	/// </summary>
	CGameWindow* m_PtrGameWindow{};
private:
	/// <summary>
	/// Информация о данном 3D обьекте (в виде массива мешей и  массива материалов)
	/// </summary>
	SModel					m_Model{};
	/// <summary>
	/// Информация о данном 3D обьекте в виде массива буфферов мешей
	/// </summary>
	vector<SMeshBuffers>	m_vMeshBuffers{};
	/// <summary>
	/// Массив указателей на текстуру модели
	/// </summary>
	vector<unique_ptr<CTexture>>	m_vEmbeddedTextures{};

	// TODO animation
	XMMATRIX						m_AnimatedBoneMatrices[KMaxBoneMatrixCount]{};
	size_t							m_CurrentAnimationIndex{};
	float							m_CurrentAnimationTick{};
};