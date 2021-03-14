#pragma once

#include <Windows.h>
#include "Object3D.h"
#include "Shader.h"
#include "PrimitiveGenerator.h"
#include "GameObject.h"

/// <summary>
/// ��� ������
/// </summary>
enum class ECameraType
{
	/// <summary>
	/// ��� ������ �� ������� ����
	/// </summary>
	FirstPerson,
	/// <summary>
	/// ��� ������ �� �������� ����
	/// </summary>
	ThirdPerson,
	/// <summary>
	/// ��� ������ ���������
	/// </summary>
	FreeLook
};

/// <summary>
/// ����������� �������� ������
/// </summary>
enum class ECameraMovementDirection
{
	/// <summary>
	/// ����������� �������� ������ ������
	/// </summary>
	Forward,
	/// <summary>
	/// ����������� �������� ������ �����
	/// </summary>
	Backward,
	/// <summary>
	/// ����������� �������� ������ ������
	/// </summary>
	Rightward,
	/// <summary>
	/// ����������� �������� ������ �����
	/// </summary>
	Leftward,
};

/// <summary>
/// ��������� ������������� (����������, ����� ����� �������������)
/// </summary>
enum class ERasterizerState
{
	/// <summary>
	/// ��� ���������� ������ (���������� �������� �� ���� ������)
	/// </summary>
	CullNone,
	/// <summary>
	/// ���������� ������ �� ������� ������� (���������� ��������, ������� ������� ������ ������ ������� �������)
	/// </summary>
	CullClockwise,
	/// <summary>
	/// ���������� ������ ������ ������� ������� (���������� ��������, ������� ������� ������ �� ������� �������, �� ���������)
	/// </summary>
	CullCounterClockwise,
	/// <summary>
	/// ���������� ������ ������ (�����) ���������
	/// </summary>
	WireFrame
};

/// <summary>
/// ���������� � ��������� ������
/// </summary>
struct SCameraData
{
	static constexpr float KDefaultDistance{ 10.0f };
	static constexpr float KDefaultMinDistance{ 1.0f };
	static constexpr float KDefaultMaxDistance{ 50.0f };

	SCameraData(ECameraType _CameraType) : CameraType{ _CameraType } {}
	SCameraData(ECameraType _CameraType, XMVECTOR _EyePosition, XMVECTOR _FocusPosition, XMVECTOR _UpDirection,
		float _Distance = KDefaultDistance, float _MinDistance = KDefaultMinDistance, float _MaxDistance = KDefaultMaxDistance) :
		CameraType{ _CameraType }, EyePosition{ _EyePosition }, FocusPosition{ _FocusPosition }, UpDirection{ _UpDirection },
		Distance{ _Distance }, MinDistance{ _MinDistance }, MaxDistance{ _MaxDistance } {}
	
	/// <summary>
	/// ��� ������
	/// </summary>
	ECameraType CameraType{};
	/// <summary>
	/// ������� ������ (������ �������)
	/// </summary>
	XMVECTOR EyePosition{ XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f) };
	/// <summary>
	/// ����� (����������� �������) ������
	/// </summary>
	XMVECTOR FocusPosition{ XMVectorSet(0.0f, 0.0f, 1.0f, 1.0f) };
	/// <summary>
	/// ������, ������������ ����� ������������ (�������) ������
	/// </summary>
	XMVECTOR UpDirection{ XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f) };

public:
	/// <summary>
	/// ������� ��������� ������� ������ �� �� ������
	/// </summary>
	float Distance{ KDefaultDistance };
	/// <summary>
	/// ����������� ��������� ������� ������ �� �� ������
	/// </summary>
	float MinDistance{ KDefaultMinDistance };
	/// <summary>
	/// ������������ ��������� ������� ������ �� �� ������
	/// </summary>
	float MaxDistance{ KDefaultMaxDistance };

public:
	/// <summary>
	/// ���� ���������� ������ �� ��������� � ��������
	/// </summary>
	float Pitch{};
	/// <summary>
	/// ���� ���������� ������ �� ����������� � ��������
	/// </summary>
	float Yaw{};
	/// <summary>
	/// ������, ������������ ������
	/// </summary>
	XMVECTOR Forward{};
};

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
	/// <param name="FontFileName">���� � ����� � ������������ �������</param>
	/// <param name="bWindowed">��������� �� ����� � ������� ������</param>
	void CreateWin32(WNDPROC WndProc, LPCTSTR WindowName, const wstring& FontFileName, bool bWindowed);

#pragma region CameraMethods
	/// <summary>
	/// ������ ������� ����� ������������� �������� m_MatrixProjection �� ������ ���� ������
	/// </summary>
	/// <param name="FOV">���� ���� ������ ������ ���� � ��������</param>
	/// <param name="NearZ">���������� �� ��������� ��������� ���������</param>
	/// <param name="FarZ">���������� �� ������� ��������� ���������</param>
	void SetPerspective(float FOV, float NearZ, float FarZ);
	/// <summary>
	/// ��������� ������ � ������ � ������ �����
	/// </summary>
	/// <param name="CameraData">����������� ������ � ������</param>
	void AddCamera(const SCameraData& CameraData);
	/// <summary>
	/// ������������� ������� ������� �������� �� ������� �����
	/// </summary>
	/// <param name="Index">������ ���������� ������ �� ������� �����</param>
	void SetCamera(size_t Index);
	/// <summary>
	/// ������� ������
	/// </summary>
	/// <param name="Direction">����������� �������� ������</param>
	/// <param name="StrideFactor">�������� (��������) �������� ������</param>
	void MoveCamera(ECameraMovementDirection Direction, float StrideFactor = 1.0f);
	/// <summary>
	/// ������������ ������ �� �������� ����
	/// </summary>
	/// <param name="DeltaX">���� �������� �� �����������</param>
	/// <param name="DeltaY">���� �������� �� ���������</param>
	/// <param name="RotationFactor">�������� (��������) �������� ������</param>
	void RotateCamera(int DeltaX, int DeltaY, float RotationFactor = 1.0f);
	/// <summary>
	/// ��������� ��������� ����� �������� � ������� ������
	/// </summary>
	/// <param name="DeltaWheel">���������� ��������� ���� ������</param>
	/// <param name="ZoomFactor">�������� (��������) ���� ������</param>
	void ZoomCamera(int DeltaWheel, float ZoomFactor = 1.0f);
#pragma endregion

#pragma region ShaderMethods
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
#pragma endregion

#pragma region Object3DMethods
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
#pragma endregion

#pragma region GameObjectMethods
	/// <summary>
	/// ������� � ���������� ��������� �� ������� ������
	/// </summary>
	/// <returns>��������� ��������� �� ������� ������</returns>
	CGameObject* AddGameObject();
	/// <summary>
	/// ��������� ��������� �� ������� ������ � ��������� ��������
	/// </summary>
	/// <param name="Index">������ ��������� � �������</param>
	/// <returns>��������� �� ������� ������ � �������� ��������</returns>
	CGameObject* GetGameObject(size_t Index);
#pragma endregion

#pragma region RenderMethods
	/// <summary>
	/// ��������� ��������� ������������� (���������, ����� ����� �������������)
	/// </summary>
	/// <param name="State">����� ��������� �������������</param>
	void SetRasterizerState(ERasterizerState State);
	/// <summary>
	/// ������� ����� �������� (������ �����) � ������ ���������� � ����������
	/// </summary>
	/// <param name="ClearColor">���� ������� ������� ����������</param>
	void BeginRendering(const FLOAT* ClearColor);
	/// <summary>
	/// ��������� ������� ������� ���� ������� �������� � ���������� ��
	/// </summary>
	void DrawGameObjects();
	/// <summary>
	/// ��������� �����������, swap ������ �������� � ������ �������
	/// </summary>
	void EndRendering();
#pragma endregion

public:

#pragma region GetMethods
	/// <summary>
	/// �������� ���������� ����
	/// </summary>
	/// <returns>���������� ����</returns>
	HWND GetHWND() { return m_hWnd; }
	/// <summary>
	/// ���������� ������� ��������� ����������
	/// </summary>
	/// <returns>������� ��������� ����������</returns>
	Keyboard::State GetKeyState();
	/// <summary>
	/// ���������� ������� ��������� ���� � ���������� �������� �������� ���������
	/// </summary>
	/// <returns>������� ��������� ����</returns>
	Mouse::State GetMouseState();
	/// <summary>
	/// ���������� ��������� ������� ��������
	/// </summary>
	/// <returns>��������� ������� ��������</returns>
	SpriteBatch* GetSpriteBatchPtr() { return m_SpriteBatch.get(); }
	/// <summary>
	/// ���������� ��������� �� ������ �������
	/// </summary>
	/// <returns>��������� �� ������ �������</returns>
	SpriteFont* GetSpriteFontPtr() { return m_SpriteFont.get(); }
#pragma endregion

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
	/// <param name="FontFileName">���� � ����� � ������������ �������</param>
	/// <param name="bWindowed">��������� �� ����� � ������� ������</param>
	void InitializeDirectX(const wstring& FontFileName, bool bWindowed);
private:

#pragma region InitD3D11ComponentMethods
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
	/// <summary>
	/// ������������� ��������� ����� (���� � ����������)
	/// </summary>
	void CreateInputDevices();
	/// <summary>
	/// ������� CBWVP (Constant Buffer World-View-Projection) ����������� ����� 
	/// </summary>
	void CreateCBWVP();
#pragma endregion

	/// <summary>
	/// ��������� ����������� ����� CBWVP (Constant Buffer World-View-Projection)
	/// </summary>
	/// <param name="MatrixWorld">������� ����</param>
	void UpdateCBWVP(const XMMATRIX& MatrixWorld);

private:
	static constexpr float KDefaultFOV{ XM_PIDIV2 };
	static constexpr float KDefaultNearZ{ 0.1f };
	static constexpr float KDefaultFarZ{ 1000.0f };
private:
	/// <summary>
	/// ������ ����������� ��������
	/// </summary>
	vector<unique_ptr<CShader>>		m_vShaders{};
	/// <summary>
	/// ������ ������������ 3D ��������
	/// </summary>
	vector<unique_ptr<CObject3D>>	m_vObject3Ds{};
	/// <summary>
	/// ������ ������������ ������� ��������
	/// </summary>
	vector<unique_ptr<CGameObject>>	m_vGameObjects{};
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
	/// <summary>
	/// ������� ��������
	/// </summary>
	XMMATRIX						m_MatrixProjection{};
	/// <summary>
	/// ������� ����
	/// </summary>
	XMMATRIX						m_MatrixView{};
	/// <summary>
	/// ������ �����
	/// </summary>
	vector<SCameraData>				m_vCameras{};
	/// <summary>
	/// ������� ������
	/// </summary>
	SCameraData*					m_PtrCurrentCamera{};
	/// <summary>
	/// ������, ������������ ������
	/// </summary>
	XMVECTOR						m_BaseForward{};
	/// <summary>
	/// ������, ������������ �����
	/// </summary>
	XMVECTOR						m_BaseUp{};
private:
	/// <summary>
	/// ������� ��������� ������������� (���������, ����� ����� ��������������)
	/// </summary>
	ERasterizerState				m_eRasterizerState{ ERasterizerState::CullCounterClockwise };
private:
	/// <summary>
	/// ��������� �� ������� ������
	/// </summary>
	ComPtr<IDXGISwapChain>			m_SwapChain{};
	/// <summary>
	/// ���������� (����������� ��� �������� � �������� � ����������� �������, �������� � ������ �����������)
	/// </summary>
	ComPtr<ID3D11Device>			m_Device{};
	/// <summary>
	/// �������� ���������� (����������� ��� ������ � ���������� ������� ������������ ���������)
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
	/// <summary>
	/// ��������� �� �������� ������ �������
	/// </summary>
	ComPtr<ID3D11Texture2D>			m_DepthStencilBuffer{};
	/// <summary>
	/// ��������� �� ����������� ����� CBWVP (Constant Buffer World-View-Projection)
	/// </summary>
	ComPtr<ID3D11Buffer>			m_CBWVP{};
	/// <summary>
	/// ��������� �� ���������� ����� ����������
	/// </summary>
	unique_ptr<Keyboard>			m_Keyboard{};
	/// <summary>
	/// ��������� �� ���������� ����� ����
	/// </summary>
	unique_ptr<Mouse>				m_Mouse{};
	/// <summary>
	/// ��������� �� ����� ��������
	/// </summary>
	unique_ptr<SpriteBatch>			m_SpriteBatch{};
	/// <summary>
	/// ��������� �� ������ ������
	/// </summary>
	unique_ptr<SpriteFont>			m_SpriteFont{};
	/// <summary>
	/// ��������� �� ����� ������� ���������� �������� (???)
	/// </summary>
	unique_ptr<CommonStates>		m_CommonStates{};
};