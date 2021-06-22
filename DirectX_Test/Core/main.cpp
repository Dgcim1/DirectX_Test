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

/*
	СТРОКА 116 и 98 AssimpLoader.h ИСПРАВИТЬ!!!!!!!!!
*/

/*
	ТУДУ

	сделать обводку призракам
	сделать генерацию и уничтожение призраков
	сделать движение призраков в сторону камеры
	сделать механику смерти
	настроить ограничивающие сферы для стен
	исправить прожекторный свет
	система частиц огня
	сделать боксы для физики столкновений стен и камеры

*/

//ОМ - этап слияния вывода, последний этап для определения видимых пикселей


static int ghostCounterMain = 0;
static int ghostCounterCurrent = 0;

static float speedCamera = 0.05f;

static EGameState startGameState = EGameState::Playing;
//static EGameState startGameState = EGameState::DebugMode;

void CreateGhost(CGameWindow& GameWindow, CObject3D* ObjectGhost, CTexture* TextureGhostColor) {
	float seed = rand() / (2 * XM_PI);
	CGameObject* CGameObjectGhost{ GameWindow.AddGameObject("Ghost" + std::to_string(ghostCounterMain)) };
	{
		CGameObjectGhost->ComponentTransform.Translation = XMVectorSet(cos(seed) * 25.0f, 0.0f, sin(seed) * 25.0f, 0.0f);
		CGameObjectGhost->ComponentTransform.Scaling = XMVectorSet(2.2f, 2.2f, 2.2f, 0.0f);
		CGameObjectGhost->UpdateWorldMatrix();

		CGameObjectGhost->ComponentRender.PtrObject3D = ObjectGhost;
		CGameObjectGhost->eFlagsGameObjectRendering = EFlagsGameObjectRendering::NoCulling | EFlagsGameObjectRendering::NoLighting;
		CGameObjectGhost->ComponentRender.PtrTexture = TextureGhostColor;
		CGameObjectGhost->ComponentRender.IsTransparent = true;
		CGameObjectGhost->ComponentPhysics.bIsPickable = true;
		CGameObjectGhost->ComponentPhysics.BoundingSphere.CenterOffset = XMVectorSet(0.0f, 1.0f, 0.0f, 0);
		CGameObjectGhost->ComponentPhysics.BoundingSphere.Radius = 3.0f;
	}
	ghostCounterMain++;
	ghostCounterCurrent++;
}

LRESULT CALLBACK WndProc(_In_ HWND hWnd, _In_ UINT Msg, _In_ WPARAM wParam, _In_ LPARAM lParam);

int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nShowCmd)
{

	srand(time(NULL));

	//создаем окно
	int windowHeight = 600;
	int windowWight = 1000;
	CGameWindow GameWindow{ hInstance, XMFLOAT2(windowWight, windowHeight) };
	GameWindow.CreateWin32(WndProc, TEXT("Game"), L"Asset\\dotumche_10_korean.spritefont", true);
	GameWindow.SetGameState(startGameState);
	//создаем и устанавливаем камеру
	if (startGameState == EGameState::DebugMode)
	{
		SCameraData MainCamera{ SCameraData(ECameraType::FreeLook) };
		{
			MainCamera.EyePosition = { 0.0f, 3.0f, 0.0f };
			MainCamera.FocusPosition = { 0.0f, 3.0f, 1.0f };
			GameWindow.AddCamera(MainCamera);
		}
	}
	if (startGameState == EGameState::Playing)
	{
		SCameraData MainCamera{ SCameraData(ECameraType::FirstPerson) };
		{
			MainCamera.EyePosition = { 0.0f, 3.0f, 0.0f };
			MainCamera.FocusPosition = { 0.0f, 3.0f, 1.0f };
			GameWindow.AddCamera(MainCamera);
		}
	}
	GameWindow.SetCamera(0);
	//устанавливаем свет
	XMVECTORF32 clightColor = Colors::White;
	XMVECTOR vlightColor = { clightColor.v };

	GameWindow.SetAmbientlLight(XMFLOAT3(clightColor), 0.15f);
	GameWindow.SetDirectionalLight(XMVectorSet(0, 1, 0, 0), vlightColor * 0.38f);
	//GameWindow.SetDirectionalLight(XMVectorSet(0, 1, 0, 0), XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f));

	XMFLOAT3 LightAttenuationRange_0{ 10.0f, 10.0f, 10.0f };
	XMFLOAT3 LightAttenuationRange_7{ 1.0f, 0.7f, 1.8f };
	XMFLOAT3 LightAttenuationRange_13{ 1.0f, 0.35f, 0.44f };
	XMFLOAT3 LightAttenuationRange_20{ 1.0f, 0.22f, 0.20f };
	XMFLOAT3 LightAttenuationRange_32{ 1.0f, 0.14f, 0.07f };
	XMFLOAT3 LightAttenuationRange_50{ 1.0f, 0.09f, 0.032f };
	XMFLOAT3 LightAttenuationRange_65{ 1.0f, 0.07f, 0.017f };
	XMFLOAT3 LightAttenuationRange_200{ 1.0f, 0.022f, 0.0019f };
	XMFLOAT3 LightAttenuationRange_3250{ 1.0f, 0.0014f, 0.000007f };

	//const float CosSpotLightCutOff = 0.57f;
	//const float outerCosSpotLightCutOff = 0.5f;
	const float CosSpotLightCutOff = 0.94f;
	const float outerCosSpotLightCutOff = 0.87f;
	const float cameraW = 0.0f;

	//GameWindow.SetPointLight(0, XMVectorSet(1, 1, 1, 0.5f), XMVectorSet(0, 0, 0, 1.0f), LightAttenuationRange_0);
	//GameWindow.SetPointLight(1, XMVectorSet(1, 1, 1, 0.5f), XMVectorSet(0, 0, 0, 1.0f), LightAttenuationRange_0);
	//GameWindow.SetPointLight(2, XMVectorSet(1, 1, 1, 0.5f), XMVectorSet(0, 0, 0, 1.0f), LightAttenuationRange_0);
	//GameWindow.SetPointLight(3, XMVectorSet(1, 1, 1, 0.5f), XMVectorSet(0, 0, 0, 1.0f), LightAttenuationRange_0);

	GameWindow.SetPointLight(0, vlightColor, XMVectorSet(8.0f, 2.7f, 0.0f, 1.0f), LightAttenuationRange_32);
	GameWindow.SetPointLight(1, vlightColor, XMVectorSet(-8.0f, 2.7f, 0.0f, 1.0f), LightAttenuationRange_32);
	GameWindow.SetPointLight(2, vlightColor, XMVectorSet(0, 0, 0, 1.0f), LightAttenuationRange_0);
	GameWindow.SetPointLight(3, vlightColor, XMVectorSet(0, 0, 0, 1.0f), LightAttenuationRange_0);

	XMVECTOR SpotlightPosition{ XMVectorSet(0, 0, 0, cameraW) };
	XMFLOAT3 SpotlightDirection{ 0, 0, 1 };
	GameWindow.SetSpotLight(
		XMVectorSet(1, 1, 1, 0.0f), //Цвет
		SpotlightPosition, //Позиция
		SpotlightDirection, //Направление света
		LightAttenuationRange_3250, //Угасание света
		CosSpotLightCutOff, //Угол, в пределах которого свет есть
		outerCosSpotLightCutOff //Угол, за пределами которого света нет
	);
	
	//GameWindow.SetDirectionalLight(XMVectorSet(1, 1, 0, 0), XMVectorSet(0, 0, 0, 1));
	//GameWindow.SetDirectionalLight(XMVectorSet(0, 100, 0, 0), XMVectorSet(1, 1, 1, 1));
	GameWindow.SetGameRenderingFlags(EFlagsGameRendering::UseLighting);
	//загружаем текстуры
	CTexture* TextureGround{ GameWindow.AddTexture() };
	{
		TextureGround->CreateFromFile(L"Asset\\ground.png");
	}
	CTexture* TextureGhostColor{ GameWindow.AddTexture() };
	{
		TextureGhostColor->CreateFromFile(L"Asset\\ghost-color.png");
	}
	//создаем 3D обьекты


	CObject3D* ObjectGhost{ GameWindow.AddObject3D() };
	{
		SModel Model{ LoadStaticModelFromFile("Asset/Floating+Cape+2.obj") };
		//SModel Model{ LoadStaticModelFromFile("Asset/succube.obj") };
		ObjectGhost->Create(Model);
	}
	CreateGhost(GameWindow, ObjectGhost, TextureGhostColor);
	
	CObject3D* ObjectDagger3{ GameWindow.AddObject3D() };
	{
		SModel Model{ LoadStaticModelFromFile("Asset/Underworld/Dagger3.obj") };
		ObjectDagger3->Create(Model);
	}
	CObject3D* ObjectDagger2{ GameWindow.AddObject3D() };
	{
		SModel Model{ LoadStaticModelFromFile("Asset/Underworld/Dagger2.obj") };
		ObjectDagger2->Create(Model);
	}
	CGameObject* CGameObjectDagger_1{ GameWindow.AddGameObject("Dagger_1") };
	{
		CGameObjectDagger_1->ComponentTransform.Translation = XMVectorSet(0.0f, -1.5f, 0.0f, 0);
		CGameObjectDagger_1->ComponentTransform.Scaling = XMVectorSet(0.15f, 0.15f, 0.15f, 0);
		CGameObjectDagger_1->UpdateWorldMatrix();
		CGameObjectDagger_1->ComponentRender.PtrObject3D = ObjectDagger3;
		CGameObjectDagger_1->ComponentRender.IsTransparent = false;
		CGameObjectDagger_1->ComponentPhysics.bIsPickable = false;
		CGameObjectDagger_1->ComponentPhysics.BoundingSphere.CenterOffset = XMVectorSet(0.0f, 1.6f, 0.0f, 0);
		CGameObjectDagger_1->ComponentPhysics.BoundingSphere.Radius = 1.8f;
	}
	CGameObject* CGameObjectDagger_2{ GameWindow.AddGameObject("Dagger_2") };
	{
		CGameObjectDagger_2->ComponentTransform.Translation = XMVectorSet(0.2f, -0.7f, 0.2f, 0);
		CGameObjectDagger_2->ComponentTransform.RotationQuaternion = XMQuaternionRotationRollPitchYaw(-XM_PIDIV2, XM_PIDIV2 / 5, -XM_PIDIV2);
		CGameObjectDagger_2->ComponentTransform.Scaling = XMVectorSet(0.15f, 0.15f, 0.15f, 0);
		CGameObjectDagger_2->UpdateWorldMatrix();
		CGameObjectDagger_2->ComponentRender.PtrObject3D = ObjectDagger2;
		CGameObjectDagger_2->ComponentRender.IsTransparent = false;
		CGameObjectDagger_2->ComponentPhysics.bIsPickable = false;
		CGameObjectDagger_2->ComponentPhysics.BoundingSphere.CenterOffset = XMVectorSet(0.0f, 1.6f, 0.0f, 0);
		CGameObjectDagger_2->ComponentPhysics.BoundingSphere.Radius = 1.8f;
	}
	CGameObject* CGameObjectDagger_3{ GameWindow.AddGameObject("Dagger_3") };
	{
		CGameObjectDagger_3->ComponentTransform.Translation = XMVectorSet(0.2f, -1.5f, 0.0f, 0);
		CGameObjectDagger_3->ComponentTransform.RotationQuaternion = XMQuaternionRotationRollPitchYaw(-XM_PIDIV2 / 8, 0, -XM_PIDIV2 / 6);
		CGameObjectDagger_3->ComponentTransform.Scaling = XMVectorSet(0.15f, 0.15f, 0.15f, 0);
		CGameObjectDagger_3->UpdateWorldMatrix();
		CGameObjectDagger_3->ComponentRender.PtrObject3D = ObjectDagger3;
		CGameObjectDagger_3->ComponentRender.IsTransparent = false;
		CGameObjectDagger_3->ComponentPhysics.bIsPickable = false;
		CGameObjectDagger_3->ComponentPhysics.BoundingSphere.CenterOffset = XMVectorSet(0.0f, 1.6f, 0.0f, 0);
		CGameObjectDagger_3->ComponentPhysics.BoundingSphere.Radius = 1.8f;
	}

	CObject3D* ObjectAltarCandlet{ GameWindow.AddObject3D() };
	{
		SModel Model{ LoadStaticModelFromFile("Asset/Underworld/AltarCandlet.obj") };
		ObjectAltarCandlet->Create(Model);
	}
	CGameObject* CGameObjectAltarCandlet_1{ GameWindow.AddGameObject("AltarCandlet_1") };
	{
		CGameObjectAltarCandlet_1->ComponentTransform.Translation = XMVectorSet(8.0f, -1.0f, 0.0f, 0);
		//CGameObjectAltarCandlet_1->ComponentTransform.RotationQuaternion = XMQuaternionRotationRollPitchYaw(-XM_PIDIV2 / 8, 0, -XM_PIDIV2 / 6);
		//CGameObjectAltarCandlet_1->ComponentTransform.Scaling = XMVectorSet(0.15f, 0.15f, 0.15f, 0);
		CGameObjectAltarCandlet_1->UpdateWorldMatrix();
		CGameObjectAltarCandlet_1->ComponentRender.PtrObject3D = ObjectAltarCandlet;
		CGameObjectAltarCandlet_1->ComponentRender.IsTransparent = false;
		CGameObjectAltarCandlet_1->ComponentPhysics.bIsPickable = false;
		CGameObjectAltarCandlet_1->ComponentPhysics.BoundingSphere.CenterOffset = XMVectorSet(0.0f, 1.6f, 0.0f, 0);
		CGameObjectAltarCandlet_1->ComponentPhysics.BoundingSphere.Radius = 1.8f;
	}

	CObject3D* ObjectWall{ GameWindow.AddObject3D() };
	{
		SModel Model{ LoadStaticModelFromFile("Asset/Underworld/wall.obj") };
		ObjectWall->Create(Model);
	}
	{
		CGameObject* CGameObjectWall_1{ GameWindow.AddGameObject("Wall_1") };
		{
			CGameObjectWall_1->ComponentTransform.Translation = XMVectorSet(15.0f, 5.0f, 12.0f, 0);
			CGameObjectWall_1->ComponentTransform.RotationQuaternion = XMQuaternionRotationRollPitchYaw(0, XM_PIDIV2, 0);
			CGameObjectWall_1->ComponentTransform.Scaling = XMVectorSet(4.0f, 4.0f, 4.0f, 0);
			CGameObjectWall_1->UpdateWorldMatrix();
			CGameObjectWall_1->ComponentRender.PtrObject3D = ObjectWall;
			CGameObjectWall_1->ComponentRender.IsTransparent = false;
			CGameObjectWall_1->ComponentPhysics.bIsPickable = true;
			CGameObjectWall_1->ComponentPhysics.BoundingSphere.CenterOffset = XMVectorSet(0.0f, 1.6f, 0.0f, 0);
			CGameObjectWall_1->ComponentPhysics.BoundingSphere.Radius = 1.8f;
		}
		CGameObject* CGameObjectWall_2{ GameWindow.AddGameObject("Wall_2") };
		{
			CGameObjectWall_2->ComponentTransform.Translation = XMVectorSet(15.0f, 5.0f, -4.0f, 0);
			CGameObjectWall_2->ComponentTransform.RotationQuaternion = XMQuaternionRotationRollPitchYaw(0, XM_PIDIV2, 0);
			CGameObjectWall_2->ComponentTransform.Scaling = XMVectorSet(4.0f, 4.0f, 4.0f, 0);
			CGameObjectWall_2->UpdateWorldMatrix();
			CGameObjectWall_2->ComponentRender.PtrObject3D = ObjectWall;
			CGameObjectWall_2->ComponentRender.IsTransparent = false;
			CGameObjectWall_2->ComponentPhysics.bIsPickable = true;
			CGameObjectWall_2->ComponentPhysics.BoundingSphere.CenterOffset = XMVectorSet(0.0f, 1.6f, 0.0f, 0);
			CGameObjectWall_2->ComponentPhysics.BoundingSphere.Radius = 1.8f;
		}
		CGameObject* CGameObjectWall_3{ GameWindow.AddGameObject("Wall_3") };
		{
			CGameObjectWall_3->ComponentTransform.Translation = XMVectorSet(12.0f, 5.0f, -15.0f, 0);
			CGameObjectWall_3->ComponentTransform.RotationQuaternion = XMQuaternionRotationRollPitchYaw(0, XM_PI, 0);
			CGameObjectWall_3->ComponentTransform.Scaling = XMVectorSet(4.0f, 4.0f, 4.0f, 0);
			CGameObjectWall_3->UpdateWorldMatrix();
			CGameObjectWall_3->ComponentRender.PtrObject3D = ObjectWall;
			CGameObjectWall_3->ComponentRender.IsTransparent = false;
			CGameObjectWall_3->ComponentPhysics.bIsPickable = true;
			CGameObjectWall_3->ComponentPhysics.BoundingSphere.CenterOffset = XMVectorSet(0.0f, 1.6f, 0.0f, 0);
			CGameObjectWall_3->ComponentPhysics.BoundingSphere.Radius = 1.8f;
		}
		CGameObject* CGameObjectWall_4{ GameWindow.AddGameObject("Wall_4") };
		{
			CGameObjectWall_4->ComponentTransform.Translation = XMVectorSet(-4.0f, 5.0f, -15.0f, 0);
			CGameObjectWall_4->ComponentTransform.RotationQuaternion = XMQuaternionRotationRollPitchYaw(0, XM_PI, 0);
			CGameObjectWall_4->ComponentTransform.Scaling = XMVectorSet(4.0f, 4.0f, 4.0f, 0);
			CGameObjectWall_4->UpdateWorldMatrix();
			CGameObjectWall_4->ComponentRender.PtrObject3D = ObjectWall;
			CGameObjectWall_4->ComponentRender.IsTransparent = false;
			CGameObjectWall_4->ComponentPhysics.bIsPickable = true;
			CGameObjectWall_4->ComponentPhysics.BoundingSphere.CenterOffset = XMVectorSet(0.0f, 1.6f, 0.0f, 0);
			CGameObjectWall_4->ComponentPhysics.BoundingSphere.Radius = 1.8f;
		}
		CGameObject* CGameObjectWall_5{ GameWindow.AddGameObject("Wall_5") };
		{
			CGameObjectWall_5->ComponentTransform.Translation = XMVectorSet(-15.0f, 5.0f, -12.0f, 0);
			CGameObjectWall_5->ComponentTransform.RotationQuaternion = XMQuaternionRotationRollPitchYaw(0, -XM_PIDIV2, 0);
			CGameObjectWall_5->ComponentTransform.Scaling = XMVectorSet(4.0f, 4.0f, 4.0f, 0);
			CGameObjectWall_5->UpdateWorldMatrix();
			CGameObjectWall_5->ComponentRender.PtrObject3D = ObjectWall;
			CGameObjectWall_5->ComponentRender.IsTransparent = false;
			CGameObjectWall_5->ComponentPhysics.bIsPickable = true;
			CGameObjectWall_5->ComponentPhysics.BoundingSphere.CenterOffset = XMVectorSet(0.0f, 1.6f, 0.0f, 0);
			CGameObjectWall_5->ComponentPhysics.BoundingSphere.Radius = 1.8f;
		}
		CGameObject* CGameObjectWall_6{ GameWindow.AddGameObject("Wall_6") };
		{
			CGameObjectWall_6->ComponentTransform.Translation = XMVectorSet(-15.0f, 5.0f, 4.0f, 0);
			CGameObjectWall_6->ComponentTransform.RotationQuaternion = XMQuaternionRotationRollPitchYaw(0, -XM_PIDIV2, 0);
			CGameObjectWall_6->ComponentTransform.Scaling = XMVectorSet(4.0f, 4.0f, 4.0f, 0);
			CGameObjectWall_6->UpdateWorldMatrix();
			CGameObjectWall_6->ComponentRender.PtrObject3D = ObjectWall;
			CGameObjectWall_6->ComponentRender.IsTransparent = false;
			CGameObjectWall_6->ComponentPhysics.bIsPickable = true;
			CGameObjectWall_6->ComponentPhysics.BoundingSphere.CenterOffset = XMVectorSet(0.0f, 1.6f, 0.0f, 0);
			CGameObjectWall_6->ComponentPhysics.BoundingSphere.Radius = 1.8f;
		}
		CGameObject* CGameObjectWall_7{ GameWindow.AddGameObject("Wall_7") };
		{
			CGameObjectWall_7->ComponentTransform.Translation = XMVectorSet(-12.0f, 5.0f, 15.0f, 0);
			CGameObjectWall_7->ComponentTransform.RotationQuaternion = XMQuaternionRotationRollPitchYaw(0, 0, 0);
			CGameObjectWall_7->ComponentTransform.Scaling = XMVectorSet(4.0f, 4.0f, 4.0f, 0);
			CGameObjectWall_7->UpdateWorldMatrix();
			CGameObjectWall_7->ComponentRender.PtrObject3D = ObjectWall;
			CGameObjectWall_7->ComponentRender.IsTransparent = false;
			CGameObjectWall_7->ComponentPhysics.bIsPickable = true;
			CGameObjectWall_7->ComponentPhysics.BoundingSphere.CenterOffset = XMVectorSet(0.0f, 1.6f, 0.0f, 0);
			CGameObjectWall_7->ComponentPhysics.BoundingSphere.Radius = 1.8f;
		}
		CGameObject* CGameObjectWall_8{ GameWindow.AddGameObject("Wall_8") };
		{
			CGameObjectWall_8->ComponentTransform.Translation = XMVectorSet(4.0f, 5.0f, 15.0f, 0);
			CGameObjectWall_8->ComponentTransform.RotationQuaternion = XMQuaternionRotationRollPitchYaw(0, 0, 0);
			CGameObjectWall_8->ComponentTransform.Scaling = XMVectorSet(4.0f, 4.0f, 4.0f, 0);
			CGameObjectWall_8->UpdateWorldMatrix();
			CGameObjectWall_8->ComponentRender.PtrObject3D = ObjectWall;
			CGameObjectWall_8->ComponentRender.IsTransparent = false;
			CGameObjectWall_8->ComponentPhysics.bIsPickable = true;
			CGameObjectWall_8->ComponentPhysics.BoundingSphere.CenterOffset = XMVectorSet(0.0f, 1.6f, 0.0f, 0);
			CGameObjectWall_8->ComponentPhysics.BoundingSphere.Radius = 1.8f;
		}
	}


	CObject3D* ObjectWallOrnament{ GameWindow.AddObject3D() };
	{
		SModel Model{ LoadStaticModelFromFile("Asset/Underworld/WallOrnament.obj") };
		ObjectWallOrnament->Create(Model);
	}
	CGameObject* CGameObjectWallOrnament_1{ GameWindow.AddGameObject("WallOrnament_1") };
	{
		CGameObjectWallOrnament_1->ComponentTransform.Translation = XMVectorSet(15.0f, -1.0f, 0.0f, 0);
		//CGameObjectWallOrnament_1->ComponentTransform.RotationQuaternion = XMQuaternionRotationRollPitchYaw(-XM_PIDIV2 / 8, 0, -XM_PIDIV2 / 6);
		CGameObjectWallOrnament_1->ComponentTransform.Scaling = XMVectorSet(1.0f, 0.95f, 1.0f, 0);
		CGameObjectWallOrnament_1->UpdateWorldMatrix();
		CGameObjectWallOrnament_1->ComponentRender.PtrObject3D = ObjectWallOrnament;
		CGameObjectWallOrnament_1->ComponentRender.IsTransparent = false;
		CGameObjectWallOrnament_1->ComponentPhysics.bIsPickable = false;
		CGameObjectWallOrnament_1->ComponentPhysics.BoundingSphere.CenterOffset = XMVectorSet(0.0f, 1.6f, 0.0f, 0);
		CGameObjectWallOrnament_1->ComponentPhysics.BoundingSphere.Radius = 1.8f;
	}

	// CObject3D* SkyBoxObject3D{ GameWindow.AddObject3D() };
	// {
	// 	SMaterial Material{ XMFLOAT3(Colors::Green) };
	// 	Material.SpecularExponent = 20.0f;
	// 	Material.SpecularIntensity = 0.8f;
	// 	SkyBoxObject3D->Create(GenerateCubeReverse(Colors::Green), Material);
	// }
	// CGameObject* SkyBoxObject{ GameWindow.AddGameObject("skybox") };
	// {
	// 	SkyBoxObject->ComponentTransform.Scaling = XMVectorSet(20.0f, 20.0f, 20.0f, 0);
	// 	SkyBoxObject->UpdateWorldMatrix();
	// 	SkyBoxObject->ComponentRender.PtrObject3D = SkyBoxObject3D;
	// }

	// CObject3D* TestPolygonObject3D{ GameWindow.AddObject3D() };
	// {
	// 	SMaterial Material{ XMFLOAT3(1.0f, 0.5f, 1.0f) };
	// 	Material.SpecularExponent = 20.0f;
	// 	Material.SpecularIntensity = 0.8f;
	// 	//TestPolygonObject3D->Create(GenerateTriangle(XMVectorSet(1, 1, 8, 1), XMVectorSet(1, 0, 8, 1), XMVectorSet(0, 1, 8, 1), Colors::Green), Material);
	// 	TestPolygonObject3D->Create(GenerateTriangle(XMVectorSet(4, 4, -4, 1), XMVectorSet(4, 0, -4, 1), XMVectorSet(0, 4, -4, 1), XMVectorSet(1, 0, 0, 0.4f)), Material);
	// }
	// CGameObject* TestPolygonObject{ GameWindow.AddGameObject("testPolygon") };
	// {
	// 	TestPolygonObject->UpdateWorldMatrix();
	// 	TestPolygonObject->ComponentRender.PtrObject3D = TestPolygonObject3D;
	// 	TestPolygonObject->eFlagsGameObjectRendering = EFlagsGameObjectRendering::NoCulling | EFlagsGameObjectRendering::NoLighting;
	// 	TestPolygonObject->ComponentRender.PtrTexture = TextureGlass;
	// 	TestPolygonObject->ComponentRender.IsTransparent = true;
	// }

	// CObject3D* SphereObject3D{ GameWindow.AddObject3D() };
	// {
	// 	SMaterial Material{ XMFLOAT3(Colors::Red) };
	// 	Material.MaterialSpecular = XMFLOAT3(Colors::White);
	// 	Material.SpecularExponent = 200.0f;
	// 	Material.SpecularIntensity = 1.8f;
	// 	SphereObject3D->Create(GenerateSphere(64, XMVectorSet(1.0f, 0.5f, 1.0f, 1)), Material);
	// }
	// CGameObject* SphereObject{ GameWindow.AddGameObject("ball") };
	// {
	// 	SphereObject->ComponentTransform.Translation = XMVectorSet(0.0f, 0.0f, +3.0f, 0);
	// 	SphereObject->UpdateWorldMatrix();
	// 	SphereObject->ComponentRender.PtrObject3D = SphereObject3D;
	// }

	CObject3D* GroundObject3D{ GameWindow.AddObject3D() };
	{
		GroundObject3D->Create(GenerateSquareXZPlane());
	}
	CGameObject* GroundObject{ GameWindow.AddGameObject("ground") };
	{
		GroundObject->ComponentTransform.Translation = XMVectorSet(0.0f, -1.0f, 0.0f, 0);
		GroundObject->ComponentTransform.Scaling = XMVectorSet(30.0f, 1.0f, 30.0f, 0);
		GroundObject->UpdateWorldMatrix();
		GroundObject->ComponentRender.PtrObject3D = GroundObject3D;
		GroundObject->ComponentRender.PtrTexture = TextureGround;

		GroundObject->ComponentPhysics.BoundingSphere.Radius = 50.0f;
	}
	
	//снеговик

	// CObject3D* SnowmanSphere1Object3D{ GameWindow.AddObject3D() };
	// {
	// 	SMaterial Material{ XMFLOAT3(Colors::White) };
	// 	Material.SpecularExponent = 2.0f;
	// 	Material.SpecularIntensity = 0.51f;
	// 	SnowmanSphere1Object3D->Create(GenerateSphere(64, Colors::White), Material);
	// }
	// CGameObject* SnowmanSphere1Object{ GameWindow.AddGameObject("snowball1") };
	// SnowmanSphere1Object->ComponentTransform.Scaling = XMVectorSet(2.0f, 2.0f, 2.0f, 0);
	// SnowmanSphere1Object->ComponentTransform.Translation = XMVectorSet(-5.0f, -0.2f, +5.0f, 0);
	// SnowmanSphere1Object->UpdateWorldMatrix();
	// SnowmanSphere1Object->ComponentRender.PtrObject3D = SnowmanSphere1Object3D;
	// 
	// CObject3D* SnowmanSphere2Object3D{ GameWindow.AddObject3D() };
	// {
	// 	SMaterial Material{ XMFLOAT3(Colors::White) };
	// 	Material.SpecularExponent = 2.0f;
	// 	Material.SpecularIntensity = 0.51f;
	// 	SnowmanSphere2Object3D->Create(GenerateSphere(64, Colors::White), Material);
	// }
	// CGameObject* SnowmanSphere2Object{ GameWindow.AddGameObject("snowball2") };
	// SnowmanSphere2Object->ComponentTransform.Scaling = XMVectorSet(1.6f, 1.6f, 1.6f, 0);
	// SnowmanSphere2Object->ComponentTransform.Translation = XMVectorSet(-5.0f, 2.2f, +5.0f, 0);
	// SnowmanSphere2Object->UpdateWorldMatrix();
	// SnowmanSphere2Object->ComponentRender.PtrObject3D = SnowmanSphere2Object3D;
	// 
	// CObject3D* SnowmanSphere3Object3D{ GameWindow.AddObject3D() };
	// {
	// 	SMaterial Material{ XMFLOAT3(Colors::White) };
	// 	Material.SpecularExponent = 2.0f;
	// 	Material.SpecularIntensity = 0.51f;
	// 	SnowmanSphere3Object3D->Create(GenerateSphere(64, Colors::White), Material);
	// }
	// CGameObject* SnowmanSphere3Object{ GameWindow.AddGameObject("snowball3") };
	// SnowmanSphere3Object->ComponentTransform.Scaling = XMVectorSet(1.3f, 1.3f, 1.3f, 0);
	// SnowmanSphere3Object->ComponentTransform.Translation = XMVectorSet(-5.0f, 4.2f, +5.0f, 0);
	// SnowmanSphere3Object->UpdateWorldMatrix();
	// SnowmanSphere3Object->ComponentRender.PtrObject3D = SnowmanSphere3Object3D;

	int rotationDeg = 0;
	bool isCaptureCursor = false;
	POINT captureCursorPos;

	//задержка для клавиш управления
	int keyPressDelay = 0;

	//лог
	bool isPrintLog = true;
	bool isPicking = false;
	bool isSpotlight = true;
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
			std::string logGameMode = "";
			switch(GameWindow.GetGameState())
			{
			case EGameState::Playing:
				logGameMode = "Gamemode: Playing\n\n";
				break;
			case EGameState::Paused:
				logGameMode = "Gamemode: Paused\n\n";
				break;
			case EGameState::GameOver:
				logGameMode = "Gamemode: GameOver\n\n";
				break;
			case EGameState::DebugMode:
				logGameMode = "Gamemode: DebugMode\n\n";
				break;
			default:
				logGameMode = "Gamemode: undefined\n\n";
				break;

			}
			
			XMFLOAT4 cameraPos;
			DirectX::XMStoreFloat4(&cameraPos, CurCamera->EyePosition);
			XMFLOAT3 cameraFocus{sin(CurCamera->Yaw), -sin(CurCamera->Pitch), cos(CurCamera->Yaw) };

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
			std::string helpF8 = "F8: Toggle visible bounding-sphere";
			std::string helpH = "H: Enable/disable flashlight";
			std::string help = "Help menu:\n\t" +
				helpF1 + "\n\t" +
				helpF2 + "\n\t" +
				helpF3 + "\n\t" +
				helpF4 + "\n\t" +
				helpF5 + "\n\t" +
				helpF6 + "\n\t" +
				helpF7 + "\n\t" +
				helpF8 + "\n\t" +
				helpH + "\n\t" +
				"\n\n";
			
			

			//лог
			if (isPicking) {
				std::string pickedObj = GameWindow.GetPickedGameObjectName();
				std::string pickInfo = "Picking obj: " + pickedObj + "\n\n";
				logInfo = logFPS + logGameMode + logCameraPos + pickInfo + help;
			}
			else {
				logInfo = logFPS + logGameMode + logCameraPos + help;
			}

			//SpotlightDirection
			//cameraFocus


			//обновление позиции фонарика
			SpotlightPosition = XMVectorSet(cameraPos.x, cameraPos.y, cameraPos.z, cameraW);
			if (isSpotlight) {
				GameWindow.SetSpotLight(
					vlightColor, //Цвет
					SpotlightPosition, //Позиция
					cameraFocus, //Направление света
					LightAttenuationRange_3250, //Угасание света
					CosSpotLightCutOff, //Угол, в пределах которого свет есть
					outerCosSpotLightCutOff //Угол, за пределами которого света нет
				);
			}
			else
			{
				GameWindow.SetSpotLight(
					vlightColor, //Цвет
					SpotlightPosition, //Позиция
					cameraFocus, //Направление света
					LightAttenuationRange_0, //Угасание света
					CosSpotLightCutOff, //Угол, в пределах которого свет есть
					outerCosSpotLightCutOff //Угол, за пределами которого света нет
				);
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
				GameWindow.MoveCamera(ECameraMovementDirection::Forward, speedCamera);
			}
			if (KeyState.S)
			{
				GameWindow.MoveCamera(ECameraMovementDirection::Backward, speedCamera);
			}
			if (KeyState.A)
			{
				GameWindow.MoveCamera(ECameraMovementDirection::Leftward, speedCamera);
			}
			if (KeyState.D)
			{
				GameWindow.MoveCamera(ECameraMovementDirection::Rightward, speedCamera);
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
				if (KeyState.F8)
				{
					GameWindow.ToggleGameRenderingFlags(EFlagsGameRendering::DrawBoundingSphere);
				}
				if (KeyState.H)
				{
					isSpotlight = !isSpotlight;
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
				if (std::string(GameWindow.GetPickedGameObjectName()).find("Ghost", 0) != std::string::npos) 
				{
					CreateGhost(GameWindow, ObjectGhost, TextureGhostColor);
				}
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
				//GameWindow.ZoomCamera(MouseState.scrollWheelValue, 0.01f);
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