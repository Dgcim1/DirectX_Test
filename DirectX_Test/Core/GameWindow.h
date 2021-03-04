#pragma once

#include <Windows.h>
#include "Object3D.h"
#include "Shader.h"

/// <summary>
/// Игровое окно
/// </summary>
class CGameWindow
{
public:
	/// <summary>
	/// Конструктор экземпляра игрового окна
	/// </summary>
	/// <param name="hInstance">Идентификатор экземпляра приложения</param>
	/// <param name="WindowSize">Размер окна</param>
	CGameWindow(HINSTANCE hInstance, const XMFLOAT2& WindowSize) : m_hInstance{ hInstance }, m_WindowSize{ WindowSize } {}

	~CGameWindow() {}

	/// <summary>
	/// Создание экземпляра окна и компонентов DirectX
	/// </summary>
	/// <param name="WndProc">Указатель на функцию - оконную процедуру</param>
	/// <param name="WindowName">Название окна</param>
	/// <param name="bWindowed">Находится ли вывод в оконном режиме</param>
	void CreateWin32(WNDPROC WndProc, LPCTSTR WindowName, bool bWindowed);
	/// <summary>
	/// Создает и возвращает указатель на шейдер
	/// </summary>
	/// <returns>Созданный указатель на шейдер</returns>
	CShader* AddShader();
	/// <summary>
	/// Получение указателя на шейдер с указанным индексом
	/// </summary>
	/// <param name="Index">Индекс указателя в массиве</param>
	/// <returns>Указатель на шейдер с заданным индексом</returns>
	CShader* GetShader(size_t Index);
	/// <summary>
	/// Создает и возвращает указатель на 3D обьект
	/// </summary>
	/// <returns>Созданный указатель на 3D обьект</returns>
	CObject3D* AddObject3D();
	/// <summary>
	/// Получение указателя на 3D обьект с указанным индексом
	/// </summary>
	/// <param name="Index">Индекс указателя в массиве</param>
	/// <returns>Указатель на 3D обьект с заданным индексом</returns>
	CObject3D* GetObject3D(size_t Index);
	/// <summary>
	/// Очищает буфер подкачки (задний буфер) заданным цветом
	/// </summary>
	/// <param name="ClearColor">Цвет заливки</param>
	void BeginRendering(const FLOAT* ClearColor);
	/// <summary>
	/// Обновляет изображение, swap буфера подкачки и буфера дисплея
	/// </summary>
	void EndRendering();
private:
	/// <summary>
	/// Инициализация окна приложения
	/// </summary>
	/// <param name="WndProc">Указатель на функцию - оконную процедуру</param>
	/// <param name="WindowName">Название окна</param>
	void CreateWin32Window(WNDPROC WndProc, LPCTSTR WindowName);
	/// <summary>
	/// Инициализация DirectX компонентов
	/// </summary>
	/// <param name="bWindowed">Находится ли вывод в оконном режиме</param>
	void InitializeDirectX(bool bWindowed);
private:
	/// <summary>
	/// Инициализация цепочки обмена и подкачки и адаптера дисплея устройства
	/// </summary>
	/// <param name="bWindowed">Находится ли вывод в оконном режиме</param>
	void CreateSwapChain(bool bWindowed);
	/// <summary>
	/// Привязывает представление заднего буфера к контексту устройства
	/// </summary>
	void CreateSetViews();
	/// <summary>
	/// Устанавливает размер области окна
	/// </summary>
	void SetViewports();
private:
	/// <summary>
	/// Массив применяемых шейдеров
	/// </summary>
	vector<unique_ptr<CShader>>		m_vShaders{};
	/// <summary>
	/// Массив используемых 3D обьектов
	/// </summary>
	vector<unique_ptr<CObject3D>>	m_vObject3Ds{};
private:
	/// <summary>
	/// Идентификатор экземпляра окна
	/// </summary>
	HWND		m_hWnd{};
	/// <summary>
	/// Идентификатор экземпляра приложения
	/// </summary>
	HINSTANCE	m_hInstance{};
	/// <summary>
	/// Размер окна
	/// </summary>
	XMFLOAT2	m_WindowSize{};

private:
	ComPtr<IDXGISwapChain>			m_SwapChain{};
	ComPtr<ID3D11Device>			m_Device{};
	/// <summary>
	/// Контекст устройства
	/// </summary>
	ComPtr<ID3D11DeviceContext>		m_DeviceContext{};
	/// <summary>
	/// Указатель на задний буфер (буфер подкачки)
	/// </summary>
	ComPtr<ID3D11RenderTargetView>	m_RenderTargetView{};
	/// <summary>
	/// Указатель на трафарет глубины
	/// </summary>
	ComPtr<ID3D11DepthStencilView>	m_DepthStencilView{};
	ComPtr<ID3D11Texture2D>			m_DepthStencilBuffer{};
};