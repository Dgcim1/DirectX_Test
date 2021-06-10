#pragma once

#include "SharedHeader.h"

/// <summary>
/// Представляет собой текстуру
/// </summary>
class CTexture
{
	friend class CGameWindow;
	friend class CObject3D;

public:
	/// <summary>
	/// Конструктор пустой текстуры
	/// </summary>
	/// <param name="PtrDevice">Указатель на устройство</param>
	/// <param name="PtrDeviceContext">Указатель на контекст устройства</param>
	CTexture(ID3D11Device* PtrDevice, ID3D11DeviceContext* PtrDeviceContext) : 
		m_PtrDevice{ PtrDevice }, m_PtrDeviceContext{ PtrDeviceContext }
	{
		assert(m_PtrDevice);
		assert(m_PtrDeviceContext);
	}
	~CTexture() {}

	/// <summary>
	/// Загрузка текстуры из файла
	/// </summary>
	/// <param name="TextureFileName">Путь к файлу</param>
	void CreateFromFile(const wstring& TextureFileName);

	/// <summary>
	/// Создание WIC (Windows Imaging Component) для текстуры и размещение его представления в RawData
	/// </summary>
	/// <param name="RawData">Массив-представление WIC</param>
	void CreateWICFromMemory(const vector<uint8_t>& RawData);
	
private:
	/// <summary>
	/// Привязка массива ресурсов шейдера и текстуры к этапу пиксельного шейдера
	/// </summary>
	void Use() const;
private:
	/// <summary>
	/// Указатель на устройство
	/// </summary>
	ID3D11Device*						m_PtrDevice{};
	/// <summary>
	/// Указатель на контекст устройства
	/// </summary>
	ID3D11DeviceContext*				m_PtrDeviceContext{};
private:
	/// <summary>
	/// Указатель на 2D текстуру
	/// </summary>
	ComPtr<ID3D11Texture2D>				m_Texture2D{};
	/// <summary>
	/// Указатель на шейдерный ресурс (подресурс, к которому шейдер может получить доступ во время рендеринга, в данном случае текстура)
	/// </summary>
	ComPtr<ID3D11ShaderResourceView>	m_ShaderResourceView{};
};