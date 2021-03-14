#pragma once

#include "SharedHeader.h"

/// <summary>
/// Вершина, состоящая из вектора позиции и цвета
/// </summary>
struct SVertex3D
{
	SVertex3D() {}
	SVertex3D(const XMVECTOR& _Position, const XMVECTOR& _Color) : Position{ _Position }, Color{ _Color } {}
	SVertex3D(const XMVECTOR& _Position, const XMVECTOR& _Color, const XMFLOAT2& _TexCoord) :
		Position{ _Position }, Color{ _Color }, TexCoord{ _TexCoord } {}
	SVertex3D(const XMVECTOR& _Position, const XMFLOAT2& _TexCoord) : Position{ _Position }, TexCoord{ _TexCoord } {}

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
	XMFLOAT2 TexCoord{};
};

/// <summary>
/// Информация о 3D обьекте, состоящая из вершин и полигонов
/// </summary>
struct SObject3DData
{
	/// <summary>
	/// Набор вершин
	/// </summary>
	vector<SVertex3D>	vVertices{};

	/// <summary>
	/// Набор полигонов (3 точки - порядковые номера массива вершин)
	/// </summary>
	vector<STriangle>	vTriangles{};
};

class CObject3D
{
public:
	/// <summary>
	/// Конструктор 3D обьекта
	/// </summary>
	/// <param name="PtrDevice">Указатель на устройство</param>
	/// <param name="PtrDeviceContext">Указатель на контекст устройства</param>
	CObject3D(ID3D11Device* PtrDevice, ID3D11DeviceContext* PtrDeviceContext) :
		m_PtrDevice{ PtrDevice }, m_PtrDeviceContext{ PtrDeviceContext }
	{
		assert(m_PtrDevice);
		assert(m_PtrDeviceContext);
	}
	~CObject3D() {}
	
	/// <summary>
	/// Создает вертексный и индексный буферы данного 3D обьекта и привязывает их к устройству 
	/// </summary>
	/// <param name="Object3DData">Указатель на данные о 3D обьекте</param>
	void Create(const SObject3DData& Object3DData);
	/// <summary>
	/// Привязываем вертексный и индексный буферы (полигоны) к конвейеру и рисуем их
	/// </summary>
	void Draw();

private:
	/// <summary>
	/// Указатель на устройство
	/// </summary>
	ID3D11Device*			m_PtrDevice{};
	/// <summary>
	/// Указатель на контекст устройства
	/// </summary>
	ID3D11DeviceContext*	m_PtrDeviceContext{};

private:
	SObject3DData			m_Object3DData{};
	/// <summary>
	/// Указатель на индексный буфер
	/// </summary>
	ComPtr<ID3D11Buffer>	m_VertexBuffer{};
	/// <summary>
	/// Указатель на массив значений шага. Каждый шаг - это размер (в байтах) элементов, которые должны использоваться из этого буфера вершин.
	/// </summary>
	UINT					m_VertexBufferStride{ sizeof(SVertex3D) };
	/// <summary>
	/// Указатель на массив значений смещения. Каждое смещение - это количество байтов между первым элементом буфера вершин и первым элементом, который будет использоваться.
	/// </summary>
	UINT					m_VertexBufferOffset{};
	/// <summary>
	/// Указатель на индексный буфер (полигоны точек из вертексного буфера)
	/// </summary>
	ComPtr<ID3D11Buffer>	m_IndexBuffer{};
};