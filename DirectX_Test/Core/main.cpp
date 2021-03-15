//было создано на основе https://github.com/principal6/DirectX113DTutorial
#include "GameWindow.h"

//IA (input-assembler) - первая часть граф конвейера

//ОМ - этап слияния вывода, последний этап для определения видимых пикселей

//структура доступа для вертексного шейдера (элемент ввода для этапа IA)
constexpr D3D11_INPUT_ELEMENT_DESC KInputElementDescs[]
{
	{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT	, 0,  0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "COLOR"	, 0, DXGI_FORMAT_R32G32B32A32_FLOAT	, 0, 16, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "TEXCOORD", 0, DXGI_FORMAT_R32G32B32A32_FLOAT	, 0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "NORMAL"	, 0, DXGI_FORMAT_R32G32B32A32_FLOAT	, 0, 48, D3D11_INPUT_PER_VERTEX_DATA, 0 },
};

LRESULT CALLBACK WndProc(_In_ HWND hWnd, _In_ UINT Msg, _In_ WPARAM wParam, _In_ LPARAM lParam);

int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nShowCmd)
{
	//создаем окно
	CGameWindow GameWindow{ hInstance, XMFLOAT2(800, 450) };
	GameWindow.CreateWin32(WndProc, TEXT("Game"), L"Asset\\dotumche_10_korean.spritefont", true);
	//создаем и устанавливаем камеру
	GameWindow.AddCamera(SCameraData(ECameraType::FreeLook));
	GameWindow.SetCamera(0);
	//создаем вертексный шейдер
	CShader* VS{ GameWindow.AddShader() };
	VS->Create(EShaderType::VertexShader, L"Shader\\VSBase.hlsl", "main", KInputElementDescs, ARRAYSIZE(KInputElementDescs));
	//создаем пиксельный шейдер
	CShader* PS{ GameWindow.AddShader() };
	PS->Create(EShaderType::PixelShader, L"Shader\\PSBase.hlsl", "main");
	//загружаем текстуры
	CTexture* TextureGround{ GameWindow.AddTexture() };
	{
		TextureGround->CreateFromFile(L"Asset\\ground.png");
	}
	//создаем 3D обьекты

	CObject3D* SkyBoxObject3D{ GameWindow.AddObject3D() };
	{
		SkyBoxObject3D->Create(GenerateCubeReverse(Colors::Green));
	}
	CGameObject* SkyBoxObject{ GameWindow.AddGameObject() };
	SkyBoxObject->ComponentTransform.Scaling = XMVectorSet(10.0f, 10.0f, 10.0f, 0);
	SkyBoxObject->UpdateWorldMatrix();
	SkyBoxObject->ComponentRender.PtrObject3D = SkyBoxObject3D;


	CObject3D* SphereObject3D{ GameWindow.AddObject3D() };
	{
		SphereObject3D->Create(GenerateSphere(64, XMVectorSet(1.0f, 0.5f, 1.0f, 1)));
	}
	CGameObject* SphereObject{ GameWindow.AddGameObject() };
	SphereObject->ComponentTransform.Translation = XMVectorSet(0.0f, 0.0f, +3.0f, 0);
	SphereObject->ComponentTransform.Rotation = XMQuaternionRotationAxis(XMVectorSet(0, 1, 0, 0), XM_PIDIV4);
	SphereObject->UpdateWorldMatrix();
	SphereObject->ComponentRender.PtrObject3D = SphereObject3D;

	CObject3D* GroundObject3D{ GameWindow.AddObject3D() };
	{
		GroundObject3D->Create(GenerateSquareXZPlane());
	}
	CGameObject* GroundObject{ GameWindow.AddGameObject() };
	GroundObject->ComponentTransform.Translation = XMVectorSet(0.0f, -1.0f, 0.0f, 0);
	GroundObject->ComponentTransform.Scaling = XMVectorSet(10.0f, 10.0f, 10.0f, 0);
	GroundObject->UpdateWorldMatrix();
	GroundObject->ComponentRender.PtrObject3D = GroundObject3D;
	GroundObject->ComponentRender.PtrTexture = TextureGround;
	

	while (true)
	{
		static MSG Msg{};
		static char key_down{};

		if (PeekMessage(&Msg, nullptr, 0, 0, PM_REMOVE))
		{
			if (Msg.message == WM_KEYDOWN) key_down = (char)Msg.wParam;
			if (Msg.message == WM_QUIT) break;

			TranslateMessage(&Msg);
			DispatchMessage(&Msg);
		}
		else
		{
			//начало рендеринга
			GameWindow.BeginRendering(Colors::Aquamarine);
			//применяем шейдеры
			VS->Use();
			PS->Use();
			//проверяем состояние клавиатуры
			Keyboard::State KeyState{ GameWindow.GetKeyState() };
			if (KeyState.Escape)
			{
				DestroyWindow(GameWindow.GetHWND());
			}
			if (KeyState.W)
			{
				GameWindow.MoveCamera(ECameraMovementDirection::Forward, 0.01f);
			}
			if (KeyState.S)
			{
				GameWindow.MoveCamera(ECameraMovementDirection::Backward, 0.01f);
			}
			if (KeyState.A)
			{
				GameWindow.MoveCamera(ECameraMovementDirection::Leftward, 0.01f);
			}
			if (KeyState.D)
			{
				GameWindow.MoveCamera(ECameraMovementDirection::Rightward, 0.01f);
			}
			if (KeyState.F1)
			{
				GameWindow.SetRasterizerState(ERasterizerState::WireFrame);
			}
			if (KeyState.F2)
			{
				GameWindow.SetRasterizerState(ERasterizerState::CullCounterClockwise);
			}
			if (KeyState.F3)
			{
				GameWindow.ToggleGameRenderingFlags(EFlagsGameRendering::DrawNormals);
			}
			//дополнительные состояния клавиатуры
			/*if (key_down == VK_F3)
			{
				GameWindow.ToggleGameRenderingFlags(EFlagsGameRendering::DrawNormals);
			}*/
			//проверяем состояние мыши
			Mouse::State MouseState{ GameWindow.GetMouseState() };
			if (MouseState.x || MouseState.y)
			{
				GameWindow.RotateCamera(MouseState.x, MouseState.y, 0.01f);
			}
			if (MouseState.scrollWheelValue)
			{
				GameWindow.ZoomCamera(MouseState.scrollWheelValue, 0.01f);
			}
			//рисуем обьекты
			GameWindow.DrawGameObjects();
			//получаем указатели на набор спрайтов и на спрайт шрифтов
			SpriteBatch* PtrSpriteBatch{ GameWindow.GetSpriteBatchPtr() };
			SpriteFont* PtrSpriteFont{ GameWindow.GetSpriteFontPtr() };
			//начало рендеринга спрайтов
			PtrSpriteBatch->Begin();
			//пишем текст
			PtrSpriteFont->DrawString(PtrSpriteBatch, "Test", XMVectorSet(0, 0, 0, 0));
			//окончание рендеринга спрайтов
			PtrSpriteBatch->End();
			//окончание рендеринга
			GameWindow.EndRendering();
		}
	}
	return 0;
}

LRESULT CALLBACK WndProc(_In_ HWND hWnd, _In_ UINT Msg, _In_ WPARAM wParam, _In_ LPARAM lParam)
{
	switch (Msg)
	{
	case WM_ACTIVATEAPP:
		Keyboard::ProcessMessage(Msg, wParam, lParam);
		break;
	case WM_INPUT:
	case WM_MOUSEMOVE:
	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_RBUTTONDOWN:
	case WM_RBUTTONUP:
	case WM_MBUTTONDOWN:
	case WM_MBUTTONUP:
	case WM_MOUSEWHEEL:
	case WM_XBUTTONDOWN:
	case WM_XBUTTONUP:
	case WM_MOUSEHOVER:
		Mouse::ProcessMessage(Msg, wParam, lParam);
		break;
	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:
	case WM_KEYUP:
	case WM_SYSKEYUP:
		Keyboard::ProcessMessage(Msg, wParam, lParam);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, Msg, wParam, lParam);
	}
	return 0;
}