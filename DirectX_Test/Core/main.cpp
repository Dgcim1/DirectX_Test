//было создано на основе https://github.com/principal6/DirectX113DTutorial
#include "GameWindow.h"

//IA (input-assembler) - перва€ часть граф конвейера

//ќћ - этап сли€ни€ вывода, последний этап дл€ определени€ видимых пикселей

//структура доступа дл€ вертексного шейдера (элемент ввода дл€ этапа IA)
constexpr D3D11_INPUT_ELEMENT_DESC KInputElementDescs[]
{
	{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0,  0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "COLOR"	, 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 16, D3D11_INPUT_PER_VERTEX_DATA, 0 },
};

LRESULT CALLBACK WndProc(_In_ HWND hWnd, _In_ UINT Msg, _In_ WPARAM wParam, _In_ LPARAM lParam);

int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nShowCmd)
{
	//создаем окно
	CGameWindow GameWindow{ hInstance, XMFLOAT2(800, 450) };
	GameWindow.CreateWin32(WndProc, TEXT("Game"), true);
	//создаем и устанавливаем камеру
	GameWindow.AddCamera(SCameraData(ECameraType::FreeLook));
	GameWindow.SetCamera(0);
	//создаем вертексный шейдер
	CShader* VS{ GameWindow.AddShader() };
	VS->Create(EShaderType::VertexShader, L"Shader\\VertexShader.hlsl", "main", KInputElementDescs, ARRAYSIZE(KInputElementDescs));
	//создаем пиксельный шейдер
	CShader* PS{ GameWindow.AddShader() };
	PS->Create(EShaderType::PixelShader, L"Shader\\PixelShader.hlsl", "main");
	//создаем 3D обьекты

	CObject3D* SkyBoxObject{ GameWindow.AddObject3D() };
	{
		SObject3DData Data{};

		//вершины текстуры
		//											X	   Y      Z    W				R     G     B    A	

		Data.vVertices.emplace_back(XMVectorSet(-10.0f, -1.0f, +10.0f, 1), XMVectorSet(0.3f, 0.8f, 0.0f, 1));// square down top    left		0
		Data.vVertices.emplace_back(XMVectorSet(+10.0f, -1.0f, +10.0f, 1), XMVectorSet(0.3f, 0.8f, 0.0f, 1));// square down top    right		1
		Data.vVertices.emplace_back(XMVectorSet(-10.0f, -1.0f, -10.0f, 1), XMVectorSet(0.3f, 1.0f, 0.3f, 1));// square down bottom left		2
		Data.vVertices.emplace_back(XMVectorSet(+10.0f, -1.0f, -10.0f, 1), XMVectorSet(0.3f, 1.0f, 0.3f, 1));// square down bottom right		3

		Data.vVertices.emplace_back(XMVectorSet(-10.0f, +10.0f, +10.0f, 1), XMVectorSet(0.3f, 0.8f, 0.0f, 1));// square up   top    left		4
		Data.vVertices.emplace_back(XMVectorSet(+10.0f, +10.0f, +10.0f, 1), XMVectorSet(0.3f, 0.8f, 0.0f, 1));// square up   top    right		5
		Data.vVertices.emplace_back(XMVectorSet(-10.0f, +10.0f, -10.0f, 1), XMVectorSet(0.3f, 1.0f, 0.3f, 1));// square up   bottom left		6
		Data.vVertices.emplace_back(XMVectorSet(+10.0f, +10.0f, -10.0f, 1), XMVectorSet(0.3f, 1.0f, 0.3f, 1));// square up   bottom right		7

		//треугольники текстуры

		Data.vTriangles.emplace_back(0, 1, 2);//down square 1
		Data.vTriangles.emplace_back(1, 3, 2);//down square 2

		Data.vTriangles.emplace_back(4, 6, 5);//up square 1
		Data.vTriangles.emplace_back(5, 6, 7);//up square 2

		Data.vTriangles.emplace_back(0, 4, 1);//top square 1
		Data.vTriangles.emplace_back(1, 4, 5);//top square 2

		Data.vTriangles.emplace_back(2, 3, 6);//bottom square 1
		Data.vTriangles.emplace_back(3, 7, 6);//bottom square 2

		Data.vTriangles.emplace_back(0, 2, 4);//left square 1
		Data.vTriangles.emplace_back(2, 6, 4);//left square 2

		Data.vTriangles.emplace_back(1, 5, 3);//left square 1
		Data.vTriangles.emplace_back(3, 5, 7);//left square 2

		SkyBoxObject->Create(Data);
	}

	CObject3D* Object{ GameWindow.AddObject3D() };
	{
		SObject3DData Data{};

		//вершины текстуры
		//								    	  X	     Y      Z    W				  R     G     B    A	

		Data.vVertices.emplace_back(XMVectorSet(-1.0f, +1.0f, +3.0f, 1), XMVectorSet(1.0f, 0.5f, 1.0f, 1));
		Data.vVertices.emplace_back(XMVectorSet(+1.0f, +1.0f, +3.0f, 1), XMVectorSet(0.5f, 1.0f, 0.5f, 1));
		Data.vVertices.emplace_back(XMVectorSet(-1.0f, -1.0f, +3.0f, 1), XMVectorSet(0.5f, 1.0f, 1.0f, 1));
		Data.vVertices.emplace_back(XMVectorSet(+1.0f, -1.0f, +3.0f, 1), XMVectorSet(0.5f, 0.5f, 0.5f, 1));

		//треугольники текстуры

		Data.vTriangles.emplace_back(0, 1, 2);//square 1
		Data.vTriangles.emplace_back(1, 3, 2);//square 2

		Object->Create(Data);
	}

	while (true)
	{
		static MSG Msg{};

		if (PeekMessage(&Msg, nullptr, 0, 0, PM_REMOVE))
		{
			if (Msg.message == WM_QUIT) break;

			TranslateMessage(&Msg);
			DispatchMessage(&Msg);
		}
		else
		{
			//начало рендеринга
			GameWindow.BeginRendering(Colors::Aquamarine);
			//примен€ем шейдеры
			VS->Use();
			PS->Use();
			//провер€ем состо€ние клавиатуры
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
			//провер€ем состо€ние мыши
			Mouse::State MouseState{ GameWindow.GetMouseState() };
			if (MouseState.x || MouseState.y)
			{
				GameWindow.RotateCamera(MouseState.x, MouseState.y, 0.01f);
			}
			if (MouseState.scrollWheelValue)
			{
				GameWindow.ZoomCamera(MouseState.scrollWheelValue, 0.01f);
			}
			//обновл€ем константный буфер
			GameWindow.UpdateCBWVP(XMMatrixIdentity());
			//рисуем обьекты
			SkyBoxObject->Draw();
			Object->Draw();
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