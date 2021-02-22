#pragma once

#include <Windows.h>
#include <d3d11.h>
#include <wrl.h>
#include "../DirectXTK/DirectXTK.h"

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "DirectXTK.lib")

using namespace Microsoft::WRL;
using namespace DirectX;

/// <summary>
/// ������� ����
/// </summary>
class CGameWindow
{
public:
	/// <summary>
	/// ����������� ������������������ ����
	/// </summary>
	/// <param name="hInstance">������������� ���������� ����������</param>
	/// <param name="WindowSize">������ ����</param>
	CGameWindow(HINSTANCE hInstance, const XMFLOAT2& WindowSize) : m_hInstance{ hInstance }, m_WindowSize{ WindowSize } {}

	~CGameWindow() {}

	/// <summary>
	/// �������� ���������� ���� � ����������� DirectX
	/// </summary>
	/// <param name="WndProc">��������� �� ������� - ������� ���������</param>
	/// <param name="WindowName">�������� ����</param>
	/// <param name="bWindowed">��������� �� ����� � ������� ������</param>
	void CreateWin32(WNDPROC WndProc, LPCTSTR WindowName, bool bWindowed);

	/// <summary>
	/// ������� ����� �������� (������ �����) �������� ������
	/// </summary>
	/// <param name="ClearColor">���� �������</param>
	void BeginRendering(const FLOAT* ClearColor);
	/// <summary>
	/// ��������� �����������, swap ������ �������� � ������ �������
	/// </summary>
	void EndRendering();
	
private:
	/// <summary>
	/// ������������� ���� ����������
	/// </summary>
	/// <param name="WndProc">��������� �� ������� - ������� ���������</param>
	/// <param name="WindowName">�������� ����</param>
	void CreateWin32Window(WNDPROC WndProc, LPCTSTR WindowName);
	/// <summary>
	/// ������������� DirectX �����������
	/// </summary>
	/// <param name="bWindowed">��������� �� ����� � ������� ������</param>
	void InitializeDirectX(bool bWindowed);

private:
	/// <summary>
	/// ������������� ������� ������ � �������� � �������� ������� ����������
	/// </summary>
	/// <param name="bWindowed">��������� �� ����� � ������� ������</param>
	void CreateSwapChain(bool bWindowed);
	/// <summary>
	/// ����������� ������������� ������� ������ � ��������� ����������
	/// </summary>
	void CreateSetRenderTargetView();
	/// <summary>
	/// ������������� ������ ������� ����
	/// </summary>
	void SetViewports();

private:
	/// <summary>
	/// ������������� ���������� ����
	/// </summary>
	HWND		m_hWnd{};
	/// <summary>
	/// ������������� ���������� ����������
	/// </summary>
	HINSTANCE	m_hInstance{};
	/// <summary>
	/// ������ ����
	/// </summary>
	XMFLOAT2	m_WindowSize{};

private:
	ComPtr<IDXGISwapChain>			m_SwapChain{};
	ComPtr<ID3D11Device>			m_Device{};
	/// <summary>
	/// �������� ����������
	/// </summary>
	ComPtr<ID3D11DeviceContext>		m_DeviceContext{};
	/// <summary>
	/// ��������� �� ������ ����� (����� ��������)
	/// </summary>
	ComPtr<ID3D11RenderTargetView>	m_RenderTargetView{};
};