//было создано на основе https://github.com/principal6/DirectX113DTutorial
#include "GameWindow.h"
#include "AssimpLoader.h"
#include <ctime>
#include <string>
#include <chrono>
#include <winuser.h>
//IA (input-assembler) - первая часть граф конвейера

/*

	m_vMiniAxisGameObjects[0]->ComponentTransform.RotationQuaternion = XMQuaternionRotationRollPitchYaw(0, 0, -XM_PIDIV2);
	m_vMiniAxisGameObjects[1]->ComponentRender.PtrObject3D = m_vMiniAxisObject3Ds[1].get();
	m_vMiniAxisGameObjects[2]->ComponentRender.PtrObject3D = m_vMiniAxisObject3Ds[2].get();
	m_vMiniAxisGameObjects[2]->ComponentTransform.RotationQuaternion = XMQuaternionRotationRollPitchYaw(0, -XM_PIDIV2, -XM_PIDIV2);


*/

//ОМ - этап слияния вывода, последний этап для определения видимых пикселей

LRESULT CALLBACK WndProc(_In_ HWND hWnd, _In_ UINT Msg, _In_ WPARAM wParam, _In_ LPARAM lParam);

int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nShowCmd)
{
	//создаем окно
	int windowHeight = 600;
	int windowWight = 1000;
	CGameWindow GameWindow{ hInstance, XMFLOAT2(windowWight, windowHeight) };
	GameWindow.CreateWin32(WndProc, TEXT("Game"), L"Asset\\dotumche_10_korean.spritefont", true);
	//создаем и устанавливаем камеру
	SCameraData MainCamera{ SCameraData(ECameraType::FreeLook) };
	{
		GameWindow.AddCamera(MainCamera);
	}
	GameWindow.SetCamera(0);
	//устанавливаем свет
	GameWindow.SetAmbientlLight(XMFLOAT3(Colors::White), 0.15f);
	//GameWindow.SetAmbientlLight(XMFLOAT3(Colors::Red), 100.15f);
	GameWindow.SetDirectionalLight(XMVectorSet(1, 1, 0, 0), XMVectorSet(1, 1, 1, 1));
	//GameWindow.SetDirectionalLight(XMVectorSet(1, 1, 0, 0), XMVectorSet(0, 0, 0, 1));
	//GameWindow.SetDirectionalLight(XMVectorSet(0, 100, 0, 0), XMVectorSet(1, 1, 1, 1));
	GameWindow.SetGameRenderingFlags(EFlagsGameRendering::UseLighting);
	//загружаем текстуры
	CTexture* TextureGround{ GameWindow.AddTexture() };
	{
		TextureGround->CreateFromFile(L"Asset\\ground.png");
	}
	//создаем 3D обьекты


	CObject3D* ObjectFarmhouse{ GameWindow.AddObject3D() };
	{
		//SModel Model{ LoadModelFromFile("Asset/farmhouse.fbx") };
		//SModel Model{ LoadModelFromFile("Asset/Underworld/AltarCandlet.obj") };
		SModel Model{ LoadStaticModelFromFile("Asset/Underworld/Dagger2.obj") };
		//SModel Model{ LoadModelFromFile("Asset/Wooden-Crates_FBX.fbx") };
		//SModel Model{ LoadModelFromFile("Asset/DarkCave_FBX.fbx") };
		ObjectFarmhouse->Create(Model);
	}
	CGameObject* goFarmhouse{ GameWindow.AddGameObject("farmhouse")};
	{
		goFarmhouse->ComponentTransform.Translation = XMVectorSet(-6.0f, 0.0f, 0.0f, 0);
		goFarmhouse->ComponentTransform.Scaling = XMVectorSet(0.2f, 0.2f, 0.2f, 0);
		goFarmhouse->UpdateWorldMatrix();

		goFarmhouse->ComponentRender.PtrObject3D = ObjectFarmhouse;
	}


	CObject3D* SkyBoxObject3D{ GameWindow.AddObject3D() };
	{
		SMaterial Material{ XMFLOAT3(Colors::Green) };
		Material.SpecularExponent = 20.0f;
		Material.SpecularIntensity = 0.8f;
		SkyBoxObject3D->Create(GenerateCubeReverse(Colors::Green), Material);
	}
	CGameObject* SkyBoxObject{ GameWindow.AddGameObject("skybox") };
	{
		SkyBoxObject->ComponentTransform.Scaling = XMVectorSet(20.0f, 20.0f, 20.0f, 0);
		SkyBoxObject->UpdateWorldMatrix();
		SkyBoxObject->ComponentRender.PtrObject3D = SkyBoxObject3D;
	}

	// CObject3D* TestPolygonObject3D{ GameWindow.AddObject3D() };
	// {
	// 	SMaterial Material{ XMFLOAT3(1.0f, 0.5f, 1.0f) };
	// 	Material.SpecularExponent = 20.0f;
	// 	Material.SpecularIntensity = 0.8f;
	// 	TestPolygonObject3D->Create(GenerateTriangle(XMVectorSet(1, 1, 8, 1), XMVectorSet(1, 0, 8, 1), XMVectorSet(0, 1, 8, 1), Colors::Green), Material);
	// }
	// CGameObject* TestPolygonObject{ GameWindow.AddGameObject("testPolygon") };
	// {
	// 	TestPolygonObject->UpdateWorldMatrix();
	// 	TestPolygonObject->ComponentRender.PtrObject3D = TestPolygonObject3D;
	// }

	CObject3D* SphereObject3D{ GameWindow.AddObject3D() };
	{
		SMaterial Material{ XMFLOAT3(Colors::Red) };
		Material.MaterialSpecular = XMFLOAT3(Colors::White);
		Material.SpecularExponent = 200.0f;
		Material.SpecularIntensity = 1.8f;
		SphereObject3D->Create(GenerateSphere(64, XMVectorSet(1.0f, 0.5f, 1.0f, 1)), Material);
	}
	CGameObject* SphereObject{ GameWindow.AddGameObject("ball") };
	{
		SphereObject->ComponentTransform.Translation = XMVectorSet(0.0f, 0.0f, +3.0f, 0);
		SphereObject->UpdateWorldMatrix();
		SphereObject->ComponentRender.PtrObject3D = SphereObject3D;
	}

	CObject3D* GroundObject3D{ GameWindow.AddObject3D() };
	{
		GroundObject3D->Create(GenerateSquareXZPlane());
	}
	CGameObject* GroundObject{ GameWindow.AddGameObject("ground") };
	{
		GroundObject->ComponentTransform.Translation = XMVectorSet(0.0f, -1.0f, 0.0f, 0);
		GroundObject->ComponentTransform.Scaling = XMVectorSet(20.0f, 1.0f, 20.0f, 0);
		GroundObject->UpdateWorldMatrix();
		GroundObject->ComponentRender.PtrObject3D = GroundObject3D;
		GroundObject->ComponentRender.PtrTexture = TextureGround;

		GroundObject->ComponentPhysics.BoundingSphere.Radius = 43.0f;
	}
	
	//снеговик

	CObject3D* SnowmanSphere1Object3D{ GameWindow.AddObject3D() };
	{
		SMaterial Material{ XMFLOAT3(Colors::White) };
		Material.SpecularExponent = 2.0f;
		Material.SpecularIntensity = 0.51f;
		SnowmanSphere1Object3D->Create(GenerateSphere(64, Colors::White), Material);
	}
	CGameObject* SnowmanSphere1Object{ GameWindow.AddGameObject("snowball1") };
	SnowmanSphere1Object->ComponentTransform.Scaling = XMVectorSet(2.0f, 2.0f, 2.0f, 0);
	SnowmanSphere1Object->ComponentTransform.Translation = XMVectorSet(-5.0f, -0.2f, +5.0f, 0);
	SnowmanSphere1Object->UpdateWorldMatrix();
	SnowmanSphere1Object->ComponentRender.PtrObject3D = SnowmanSphere1Object3D;

	CObject3D* SnowmanSphere2Object3D{ GameWindow.AddObject3D() };
	{
		SMaterial Material{ XMFLOAT3(Colors::White) };
		Material.SpecularExponent = 2.0f;
		Material.SpecularIntensity = 0.51f;
		SnowmanSphere2Object3D->Create(GenerateSphere(64, Colors::White), Material);
	}
	CGameObject* SnowmanSphere2Object{ GameWindow.AddGameObject("snowball2") };
	SnowmanSphere2Object->ComponentTransform.Scaling = XMVectorSet(1.6f, 1.6f, 1.6f, 0);
	SnowmanSphere2Object->ComponentTransform.Translation = XMVectorSet(-5.0f, 2.2f, +5.0f, 0);
	SnowmanSphere2Object->UpdateWorldMatrix();
	SnowmanSphere2Object->ComponentRender.PtrObject3D = SnowmanSphere2Object3D;

	CObject3D* SnowmanSphere3Object3D{ GameWindow.AddObject3D() };
	{
		SMaterial Material{ XMFLOAT3(Colors::White) };
		Material.SpecularExponent = 2.0f;
		Material.SpecularIntensity = 0.51f;
		SnowmanSphere3Object3D->Create(GenerateSphere(64, Colors::White), Material);
	}
	CGameObject* SnowmanSphere3Object{ GameWindow.AddGameObject("snowball3") };
	SnowmanSphere3Object->ComponentTransform.Scaling = XMVectorSet(1.3f, 1.3f, 1.3f, 0);
	SnowmanSphere3Object->ComponentTransform.Translation = XMVectorSet(-5.0f, 4.2f, +5.0f, 0);
	SnowmanSphere3Object->UpdateWorldMatrix();
	SnowmanSphere3Object->ComponentRender.PtrObject3D = SnowmanSphere3Object3D;

	int rotationDeg = 0;
	bool isCaptureCursor = false;
	POINT captureCursorPos;

	//задержка для клавиш управления
	int keyPressDelay = 0;

	//лог
	bool isPrintLog = true;
	bool isPicking = false;
	std::string logInfo = "";
	
	clock_t oldTime = clock();
	clock_t deltaTime = oldTime;

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
			static std::chrono::steady_clock Clock{};
			long long TimeNow{ Clock.now().time_since_epoch().count() };
			static long long TimePrev{ TimeNow };
			long long DeltaTimeLL{ TimeNow - TimePrev };
			float DeltaTimeF{ DeltaTimeLL * 0.000'000'001f };

			deltaTime = clock() - oldTime;
			int fps = (1.0 / deltaTime) * 1000;
			oldTime = clock();

		 	SCameraData* CurCamera = GameWindow.GetCurrentCamera();

			std::string logFPS = "FPS: " + std::to_string(fps) + "\n\n";
			XMFLOAT4 cameraPos;
			DirectX::XMStoreFloat4(&cameraPos, CurCamera->EyePosition);
			std::string logCameraPos = "Camera pos:\n\tx: " + std::to_string(cameraPos.x) + 
				"\n\ty: " + std::to_string(cameraPos.y) +
				"\n\tz: " + std::to_string(cameraPos.z) +
				"\n\tvertical: " + std::to_string(CurCamera->Pitch) +
				"\n\thorizontal: " + std::to_string(CurCamera->Yaw) +
				"\n\n";

			std::string helpF1 = "F1: WireFrame rasteriser";
			std::string helpF2 = "F2: Normal rasteriser";
			std::string helpF3 = "F3: Toggle render normals";
			std::string helpF4 = "F4: Toggle render light";
			std::string helpF5 = "F5: Toggle mini axes visible";
			std::string helpF6 = "F6: Toggle log visible";
			std::string helpF7 = "F7: Toggle picking visible";
			std::string help = "Help menu:\n\t" +
				helpF1 + "\n\t" +
				helpF2 + "\n\t" +
				helpF3 + "\n\t" +
				helpF4 + "\n\t" +
				helpF5 + "\n\t" +
				helpF6 + "\n\t" +
				helpF7 + "\n\t" +
				"\n\n";
			
			

			//лог
			if (isPicking) {
				std::string pickedObj = GameWindow.GetPickedGameObjectName();
				std::string pickInfo = "Picking obj: " + pickedObj + "\n\n";
				logInfo = logFPS + logCameraPos + pickInfo + help;
			}
			else {
				logInfo = logFPS + logCameraPos + help;
			}

			//начало рендеринга
			GameWindow.BeginRendering(Colors::DarkGray);
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
			if (keyPressDelay == 0) {
				keyPressDelay = fps / 10;
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
					keyPressDelay = fps / 7;
				}
				if (KeyState.F4)
				{
					GameWindow.ToggleGameRenderingFlags(EFlagsGameRendering::UseLighting);
				}
				if (KeyState.F5)
				{
					GameWindow.ToggleGameRenderingFlags(EFlagsGameRendering::DrawMiniAxes);
				}
				if (KeyState.F6) 
				{
					isPrintLog = !isPrintLog;
				}
				if (KeyState.F7)
				{
					GameWindow.ToggleGameRenderingFlags(EFlagsGameRendering::DrawPickingData);
				}
			}
			else
			{
				keyPressDelay--;
			}
			//проверяем состояние мыши
			Mouse::State MouseState{ GameWindow.GetMouseState() };
			static int MouseX{ MouseState.x };//предыдущая позиция курсора по X
			static int MouseY{ MouseState.y };//предыдущая позиция курсора по Y
			if (MouseState.leftButton)
			{
				isPicking = true;
				GameWindow.Pick(windowWight / 2, windowHeight / 2);
			}
			if (MouseState.x != MouseX || MouseState.y != MouseY)
			{
				GameWindow.RotateCamera(MouseState.x - MouseX, MouseState.y - MouseY, 0.01f);
				//привязка окна к курсору
				if (isCaptureCursor) {
					SetCursorPos(captureCursorPos.x, captureCursorPos.y);
				}
				else
				{
					GetCursorPos(&captureCursorPos);
					isCaptureCursor = true;
					//обновление координат
					MouseX = MouseState.x;
					MouseY = MouseState.y;
				}
			}
			if (MouseState.scrollWheelValue)
			{
				GameWindow.ZoomCamera(MouseState.scrollWheelValue, 0.01f);
			}
			//рисуем обьекты
			GameWindow.AnimateGameObjects();
			GameWindow.DrawGameObjects(DeltaTimeF);
			//получаем указатели на набор спрайтов и на спрайт шрифтов
			SpriteBatch* PtrSpriteBatch{ GameWindow.GetSpriteBatchPtr() };
			SpriteFont* PtrSpriteFont{ GameWindow.GetSpriteFontPtr() };
			//начало рендеринга спрайтов
			PtrSpriteBatch->Begin();
			if (isPrintLog) {
				//пишем текст
				PtrSpriteFont->DrawString(PtrSpriteBatch, logInfo.c_str(), XMVectorSet(0, 0, 0, 0));
			}
			//окончание рендеринга спрайтов
			PtrSpriteBatch->End();
			//окончание рендеринга
			GameWindow.EndRendering();

			TimePrev = TimeNow;
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