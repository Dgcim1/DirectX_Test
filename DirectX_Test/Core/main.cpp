//было создано на основе https://github.com/principal6/DirectX113DTutorial
#include "GameWindow.h"

//структура доступа для вертексного шейдера
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
	//создаем вертексный шейдер
	CShader* VS{ GameWindow.AddShader() };
	VS->Create(EShaderType::VertexShader, L"Shader\\VertexShader.hlsl", "main", KInputElementDescs, ARRAYSIZE(KInputElementDescs));
	//создаем пиксельный шейдер
	CShader* PS{ GameWindow.AddShader() };
	PS->Create(EShaderType::PixelShader, L"Shader\\PixelShader.hlsl", "main");
	//создаем 3D обьект
	CObject3D* Object{ GameWindow.AddObject3D() };
	{
		SObject3DData Data{};

		//вершины текстуры

		//Data.vVertices.emplace_back(XMVectorSet(-0.5f, +0.5f, 0, 1), XMVectorSet(1.0f, 0.5f, 1.0f, 1));
		Data.vVertices.emplace_back(XMVectorSet(-0.1f, +0.5f, 0, 1), XMVectorSet(1.0f, 0.0f, 1.0f, 1));
		//Data.vVertices.emplace_back(XMVectorSet(+0.5f, +0.5f, 0, 1), XMVectorSet(0.5f, 1.0f, 0.5f, 1));
		Data.vVertices.emplace_back(XMVectorSet(+0.5f, +0.5f, 0, 1), XMVectorSet(0.0f, 1.0f, 0.0f, 1));
		//Data.vVertices.emplace_back(XMVectorSet(-0.5f, -0.5f, 0, 1), XMVectorSet(0.5f, 1.0f, 1.0f, 1));
		Data.vVertices.emplace_back(XMVectorSet(-0.5f, -0.5f, 0, 1), XMVectorSet(0.0f, 1.0f, 0.0f, 1));
		//Data.vVertices.emplace_back(XMVectorSet(+0.5f, -0.5f, 0, 1), XMVectorSet(0.5f, 0.5f, 0.5f, 1));
		Data.vVertices.emplace_back(XMVectorSet(+0.5f, -0.5f, 0, 1), XMVectorSet(1.0f, 0.0f, 1.0f, 1));

		//треугольники текстуры

		Data.vTriangles.emplace_back(0, 1, 2);
		Data.vTriangles.emplace_back(0, 2, 1);
		Data.vTriangles.emplace_back(1, 3, 2);
		Data.vTriangles.emplace_back(1, 2, 3);

		//создаем текстуру
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
			GameWindow.BeginRendering(Colors::Red);

			VS->Use();
			PS->Use();

			Object->Draw();

			GameWindow.EndRendering();
		}
	}
	return 0;
}

LRESULT CALLBACK WndProc(_In_ HWND hWnd, _In_ UINT Msg, _In_ WPARAM wParam, _In_ LPARAM lParam)
{
	switch (Msg)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	default:
		break;
	}
	return DefWindowProc(hWnd, Msg, wParam, lParam);
}