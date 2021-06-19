#pragma once

#include <d3dcompiler.h>
#include "SharedHeader.h"

#pragma comment(lib, "d3dcompiler.lib")

/// <summary>
/// Шейдер
/// </summary>
class CShader final
{
	/// <summary>
	/// Константный буфер
	/// </summary>
	class CConstantBuffer
	{
	public:
		/// <summary>
		/// Конструктор - устанавливает устройство и контекст
		/// </summary>
		/// <param name="PtrDevice">Указатель на устройство</param>
		/// <param name="PtrDeviceContext">Указатель на контекст устройства</param>
		CConstantBuffer(ID3D11Device* PtrDevice, ID3D11DeviceContext* PtrDeviceContext) :
			m_PtrDevice{ PtrDevice }, m_PtrDeviceContext{ PtrDeviceContext }
		{
			assert(m_PtrDevice);
			assert(m_PtrDeviceContext);
		}
		~CConstantBuffer() {}

	public:
		/// <summary>
		/// Создание константного буфера
		/// </summary>
		/// <param name="ShaderType">Тип шейдера</param>
		/// <param name="PtrData">Указатель на данные</param>
		/// <param name="DataByteWidth">Обьем данных в байтах</param>
		void Create(EShaderType ShaderType, const void* PtrData, size_t DataByteWidth);
		/// <summary>
		/// Обновление константного буфера
		/// </summary>
		void Update();
		/// <summary>
		/// Установка константного буфера в видеопамять
		/// </summary>
		/// <param name="Slot">Индекс регистра константного буфера в видеопамяти</param>
		void Use(UINT Slot) const;

	private:
		/// <summary>
		/// Указатель на устройство
		/// </summary>
		ID3D11Device* m_PtrDevice{};
		/// <summary>
		/// Указатель на контекст устройства
		/// </summary>
		ID3D11DeviceContext* m_PtrDeviceContext{};

	private:
		/// <summary>
		/// Указатель на константный буфер (который загружается в видеопамять)
		/// </summary>
		ComPtr<ID3D11Buffer>	m_ConstantBuffer{};
		/// <summary>
		/// Тип шейдера
		/// </summary>
		EShaderType				m_eShaderType{};
		/// <summary>
		/// Размер буфера
		/// </summary>
		size_t					m_DataByteWidth{};
		/// <summary>
		/// Указатель на данные
		/// </summary>
		const void* m_PtrData{};
	};


public:
	/// <summary>
	/// Конструктор шейдера - устанавливает устройство и контекст
	/// </summary>
	/// <param name="PtrDevice">Указатель на устройство</param>
	/// <param name="PtrDeviceContext">Указатель на контекст устройства</param>
	CShader(ID3D11Device* PtrDevice, ID3D11DeviceContext* PtrDeviceContext) :
		m_PtrDevice{ PtrDevice }, m_PtrDeviceContext{ PtrDeviceContext }
	{
		assert(m_PtrDevice); 
		assert(m_PtrDeviceContext); 
	}
	~CShader() {}
	/// <summary>
	/// Компилирует и добавляет в устройство заданный шейдер
	/// </summary>
	/// <param name="Type">Тип шейдера</param>
	/// <param name="FileName">Путь к файлу *.hlsl с кодом шейдера</param>
	/// <param name="EntryPoint">Имя функции - точки входа в шейдер</param>
	/// <param name="InputElementDescs">Указатель на массив - элемент ввода для этапа IA</param>
	/// <param name="NumElements">Количество типов входных данных в массиве входных элементов.</param>
	void Create(EShaderType Type, const wstring& FileName, const string& EntryPoint, 
		const D3D11_INPUT_ELEMENT_DESC* InputElementDescs = nullptr, UINT NumElements = 0);
	/// <summary>
	/// Добавление константного буфера к шейдеру
	/// </summary>
	/// <param name="PtrData">Указатель на структуру данных константного буфера</param>
	/// <param name="DataByteWidth">Длина структуры в байтах</param>
	void AddConstantBuffer(const void* PtrData, size_t DataByteWidth);
	/// <summary>
	/// Обновляет константный буфер
	/// </summary>
	/// <param name="ConstantBufferIndex">Индекс константного буфера</param>
	void UpdateConstantBuffer(size_t ConstantBufferIndex);
	/// <summary>
	/// Обновляет все привязанные к шейдеру константные буферы
	/// </summary>
	void UpdateAllConstantBuffers();

	/// <summary>
	/// Устанавливает шейдер для контекста устройства
	/// </summary>
	void Use();

private:
	/// <summary>
	/// указатель на устройство
	/// </summary>
	ID3D11Device*				m_PtrDevice{};
	/// <summary>
	/// указатель на контекст устройства
	/// </summary>
	ID3D11DeviceContext*		m_PtrDeviceContext{};

private:
	/// <summary>
	/// указатель на скомпилированный код шейдера
	/// </summary>
	ComPtr<ID3DBlob>			m_Blob{};
	/// <summary>
	/// Указатель на скомпилированный вертексный шейдер (если m_ShaderType = VertexShader)
	/// </summary>
	ComPtr<ID3D11VertexShader>	m_VertexShader{};
	/// <summary>
	/// Указатель на скомпилированный пиксельный шейдер (если m_ShaderType = PixelShader)
	/// </summary>
	ComPtr<ID3D11PixelShader>	m_PixelShader{};
	/// <summary>
	/// Указатель на скомпилированный геометрический шейдер (если m_ShaderType = GeometryShader)
	/// </summary>
	ComPtr<ID3D11GeometryShader>	m_GeometryShader{};
	/// <summary>
	/// Обьект макета ввода для описания данных буфера ввода на этапе ввода-ассемблера
	/// </summary>
	ComPtr<ID3D11InputLayout>	m_InputLayout{};
	/// <summary>
	/// Тип шейдера
	/// </summary>
	EShaderType					m_ShaderType{};
	/// <summary>
	/// Вектор константных буферов шейдера
	/// </summary>
	vector<unique_ptr<CConstantBuffer>>	m_vConstantBuffers{};
};