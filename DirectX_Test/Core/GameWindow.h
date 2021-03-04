#pragma once

#include <Windows.h>
#include "Object3D.h"
#include "Shader.h"

/// <summary>
/// ������� ����
/// </summary>
class CGameWindow
{
public:
	/// <summary>
	/// ����������� ���������� �������� ����
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
	/// ������� � ���������� ��������� �� ������
	/// </summary>
	/// <returns>��������� ��������� �� ������</returns>
	CShader* AddShader();
	/// <summary>
	/// ��������� ��������� �� ������ � ��������� ��������
	/// </summary>
	/// <param name="Index">������ ��������� � �������</param>
	/// <returns>��������� �� ������ � �������� ��������</returns>
	CShader* GetShader(size_t Index);
	/// <summary>
	/// ������� � ���������� ��������� �� 3D ������
	/// </summary>
	/// <returns>��������� ��������� �� 3D ������</returns>
	CObject3D* AddObject3D();
	/// <summary>
	/// ��������� ��������� �� 3D ������ � ��������� ��������
	/// </summary>
	/// <param name="Index">������ ��������� � �������</param>
	/// <returns>��������� �� 3D ������ � �������� ��������</returns>
	CObject3D* GetObject3D(size_t Index);
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
	void CreateSetViews();
	/// <summary>
	/// ������������� ������ ������� ����
	/// </summary>
	void SetViewports();
private:
	/// <summary>
	/// ������ ����������� ��������
	/// </summary>
	vector<unique_ptr<CShader>>		m_vShaders{};
	/// <summary>
	/// ������ ������������ 3D ��������
	/// </summary>
	vector<unique_ptr<CObject3D>>	m_vObject3Ds{};
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
	/// <summary>
	/// ��������� �� �������� �������
	/// </summary>
	ComPtr<ID3D11DepthStencilView>	m_DepthStencilView{};
	ComPtr<ID3D11Texture2D>			m_DepthStencilBuffer{};
};