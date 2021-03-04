#pragma once

#include <d3dcompiler.h>
#include "SharedHeader.h"

#pragma comment(lib, "d3dcompiler.lib")

/// <summary>
/// Тип шейдера
/// </summary>
enum class EShaderType
{
	VertexShader,
	PixelShader,
};

/// <summary>
/// Шейдер
/// </summary>
class CShader final
{
public:
	/// <summary>
	/// Конструктор шейдера
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
	ComPtr<ID3D11InputLayout>	m_InputLayout{};
	/// <summary>
	/// Тип шейдера
	/// </summary>
	EShaderType					m_ShaderType{};
};