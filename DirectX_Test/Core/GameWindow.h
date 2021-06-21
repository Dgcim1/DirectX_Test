#pragma once

#define NOMINMAX 0

#include <Windows.h>
#include "Object3D.h"
#include "Shader.h"
#include "PrimitiveGenerator.h"
#include "GameObject.h"
#include "Math.h"
#include "ObjectLine.h"

/// <summary>
/// Флаги отрисовки
/// </summary>
enum class EFlagsGameRendering
{
	/// <summary>
	/// Пустой флаг
	/// </summary>
	None = 0x00,
	/// <summary>
	/// Отрисовываем нормали
	/// </summary>
	DrawNormals = 0x01,
	/// <summary>
	/// Используется ли освещение
	/// </summary>
	UseLighting = 0x02,
	/// <summary>
	/// Отрисовка мини осей
	/// </summary>
	DrawMiniAxes = 0x04,
	/// <summary>
	/// Отрисовка инфо о picking'е (луч трассировки)
	/// </summary>
	DrawPickingData = 0x08,
	/// <summary>
	/// Отрисовка ограничивающей сферы
	/// </summary>
	DrawBoundingSphere = 0x10
};
ENUM_CLASS_FLAG(EFlagsGameRendering)

/// <summary>
/// Тип камеры
/// </summary>
enum class ECameraType
{
	/// <summary>
	/// Тип камеры от первого лица
	/// </summary>
	FirstPerson,
	/// <summary>
	/// Тип камеры от третьего лица
	/// </summary>
	ThirdPerson,
	/// <summary>
	/// Тип камеры свободная
	/// </summary>
	FreeLook
};

/// <summary>
/// Направление движения камеры
/// </summary>
enum class ECameraMovementDirection
{
	/// <summary>
	/// Направление движения камеры вперед
	/// </summary>
	Forward,
	/// <summary>
	/// Направление движения камеры назад
	/// </summary>
	Backward,
	/// <summary>
	/// Направление движения камеры вправо
	/// </summary>
	Rightward,
	/// <summary>
	/// Направление движения камеры влево
	/// </summary>
	Leftward,
};

/// <summary>
/// Состояния растеризатора (показывает, какие грани отбраковывать)
/// </summary>
enum class ERasterizerState
{
	/// <summary>
	/// Без отбраковки граней (отображает полигоны со всех сторон)
	/// </summary>
	CullNone,
	/// <summary>
	/// Отбраковка граней по часовой стрелке (отображает полигоны, вершины которых заданы против часовой стрелки)
	/// </summary>
	CullClockwise,
	/// <summary>
	/// Отбраковка граней против часовой стрелки (отображает полигоны, вершины которых заданы по часовой стрелке, по умолчанию)
	/// </summary>
	CullCounterClockwise,
	/// <summary>
	/// Отображает только каркас (ребра) полигонов
	/// </summary>
	WireFrame
};

/// <summary>
/// Информация о состоянии камеры
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
	/// Тип камеры
	/// </summary>
	ECameraType CameraType{};
	/// <summary>
	/// Позиция камеры (откуда смотрит)
	/// </summary>
	XMVECTOR EyePosition{ XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f) };
	/// <summary>
	/// Фокус (направление взгляда) камеры
	/// </summary>
	XMVECTOR FocusPosition{ XMVectorSet(0.0f, 0.0f, 1.0f, 1.0f) };
	/// <summary>
	/// Вектор, направленный вверх относительно (взгляда) камеры
	/// </summary>
	XMVECTOR UpDirection{ XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f) };

public:
	/// <summary>
	/// Текущее отдаление позиции камеры от ее фокуса
	/// </summary>
	float Distance{ KDefaultDistance };
	/// <summary>
	/// Минимальное отдаление позиции камеры от ее фокуса
	/// </summary>
	float MinDistance{ KDefaultMinDistance };
	/// <summary>
	/// Максимальное отдаление позиции камеры от ее фокуса
	/// </summary>
	float MaxDistance{ KDefaultMaxDistance };

public:
	/// <summary>
	/// Угол отклонения камеры по вертикали в радианах
	/// </summary>
	float Pitch{};
	/// <summary>
	/// Угол отклонения камеры по горизонтали в радианах
	/// </summary>
	float Yaw{};
	/// <summary>
	/// Вектор, направленный вперед
	/// </summary>
	XMVECTOR Forward{};
};

/// <summary>
/// Структура привязки данных константного буфера для вертексного шейдера
/// </summary>
struct SCBVSSpaceData
{
	/// <summary>
	/// Матрица World-View-Projection
	/// </summary>
	XMMATRIX WVP{};
	/// <summary>
	/// Мировая матрица
	/// </summary>
	XMMATRIX World{};
};

/// <summary>
/// TODO структура анимации (не используется)
/// </summary>
struct SCBVSAnimationBonesData
{
	XMMATRIX	BoneMatrices[KMaxBoneMatrixCount]{};
};

/// <summary>
/// Структура привязки данных константного буфера (флаги) для пиксельного шейдера
/// </summary>
struct SCBPSBaseFlagsData
{
	/// <summary>
	/// Используется ли текстура для данного пикселя
	/// </summary>
	BOOL bUseTexture{};
	/// <summary>
	/// Используется ли освещение для данного пикселя
	/// </summary>
	BOOL bUseLighting{};
	/// <summary>
	/// Пока не используется (для выравнивания)
	/// </summary>
	BOOL Pad[2]{};
};

/// <summary>
/// Структура привязки данных константного буфера (базовое освещение) для пиксельного шейдера
/// </summary>
struct SCBPSBaseLightsData
{
	/// <summary>
	/// Направление света, положение источника света
	/// </summary>
	XMVECTOR	DirectionalLightDirection{ XMVectorSet(0, 1, 0, 0) };
	/// <summary>
	/// Цвет направленного света от источника
	/// </summary>
	XMVECTOR	DirectionalColor{ XMVectorSet(1, 1, 1, 1) };
	/// <summary>
	/// Цвет минимальной яркости мира
	/// </summary>
	XMFLOAT3	AmbientLightColor{ 1, 1, 1 };
	/// <summary>
	/// Интенсивность света минимальной яркости мира
	/// </summary>
	float		AmbientLightIntensity{ 0.5f };
};

/// <summary>
/// Структура привязки данных константного буфера (прожекторное освещение, фонарик) для пиксельного шейдера
/// </summary>
struct SCBPSSpotLightsData
{
	/// <summary>
	/// Цвет источника света
	/// </summary>
	XMVECTOR	SpotlightColor{ XMVectorSet(1.0f, 1.0f, 1.0f, 0.0f) };
	/// <summary>
	/// Позиция источника света
	/// </summary>
	XMVECTOR	SpotlightPosition{ XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f) };
	/// <summary>
	/// Вектор направления света (единичный, из 0)
	/// </summary>
	XMFLOAT3	SpotlightDirection{ 0.0f, 0.0f, 1.0f };
	/// <summary>
	/// Вектор угасания света с расстоянием
	/// </summary>
	XMFLOAT3	SpotlightAttenuation{ 1.0f, 0.14f, 0.07f };
	/// <summary>
	/// Зона 100% освещенности (угол в виде косинуса)
	/// </summary>
	float		SpotlightCutOffCosin{ 0.57f };
	/// <summary>
	/// Зона, за пределами которого нет освещенности (угол в виде косинуса)
	/// </summary>
	float		SpotlightOuterCutOffCosin{ 0.5f };
};

/// <summary>
/// Структура привязки данных константного буфера (точечное освещение) для пиксельного шейдера
/// </summary>
struct SCBPSPointLightsData
{
	/// <summary>
	/// Цвет источника света
	/// </summary>
	XMVECTOR	PointlightColor{ XMVectorSet(1, 1, 1, 0.5) };
	/// <summary>
	/// Позиция источника света
	/// </summary>
	XMVECTOR	PointlightPosition{ XMVectorSet(0, 0, 0, 1.0f) };
	/// <summary>
	/// Вектор угасания света с расстоянием
	/// </summary>
	XMFLOAT3		PointlightAttenuation{ 0, 0, 0 };
	/// <summary>
	/// Не используется
	/// </summary>
	float	Pads{ 0 };
};


/// <summary>
/// Структура привязки данных константного буфера (материал) для пиксельного шейдера
/// </summary>
struct SCBPSBaseMaterialData
{
	/// <summary>
	/// Цвет материала (затененной части обьекта)
	/// </summary>
	XMFLOAT3	MaterialAmbient{};
	/// <summary>
	/// Зеркальная экспонента (размер блика, чем больше значение, тем меньше блик и более размытые границы)
	/// </summary>
	float		SpecularExponent{ 1 };
	/// <summary>
	/// Цвет материала (освещенной части материала)
	/// </summary>
	XMFLOAT3	MaterialDiffuse{};
	/// <summary>
	/// Зеркальная интенсивность (насколько интенсивно/ярко светит отраженный от обьекта бликующий свет)
	/// </summary>
	float		SpecularIntensity{ 0 };
	/// <summary>
	/// Цвет материала (цвет блика)
	/// </summary>
	XMFLOAT3	MaterialSpecular{};
	/// <summary>
	/// Неиспользуемая переменная (для выравнивания)  
	/// </summary>
	float		Pad{};
};

/// <summary>
/// Структура текущего времени
/// </summary>
struct SCBPSSkyTimeData
{
	/// <summary>
	/// Текущее время
	/// </summary>
	float	SkyTime{};
	/// <summary>
	/// Не используется
	/// </summary>
	float	Pads[3]{};
};

/// <summary>
/// Структура привязки данных константного буфера (позиция камеры) для пиксельного шейдера
/// </summary>
struct SCBPSBaseEyeData
{
	XMVECTOR EyePosition{};
};

/// <summary>
/// Игровое окно
/// </summary>
class CGameWindow
{
	friend class CObject3D;
	friend class CGameObject;// TODO возможно удалить за ненадобностью
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
	/// <param name="FontFileName">Путь к файлу с используемым шрифтом</param>
	/// <param name="bWindowed">Находится ли вывод в оконном режиме</param>
	void CreateWin32(WNDPROC WndProc, LPCTSTR WindowName, const wstring& FontFileName, bool bWindowed);

#pragma region SetGameRenderPropertyMethods
	/// <summary>
	/// Строит матрицу левой перспективной проекции m_MatrixProjection на основе поля зрения
	/// </summary>
	/// <param name="FOV">Угол поля зрения сверху вниз в радианах</param>
	/// <param name="NearZ">Расстояние до ближайшей плоскости остечения</param>
	/// <param name="FarZ">Расстояние до дальней плоскости отсечения</param>
	void SetPerspective(float FOV, float NearZ, float FarZ);
	/// <summary>
	/// Установка указанного флага отрисовки
	/// </summary>
	/// <param name="Flags">Устанавливаемое значение флага отрисовки</param>
	void SetGameRenderingFlags(EFlagsGameRendering Flags);
	/// <summary>
	/// Переключение указанного флага отрисовки на противоположный
	/// </summary>
	/// <param name="Flags">Переключаемый флаг</param>
	void ToggleGameRenderingFlags(EFlagsGameRendering Flags);
#pragma endregion

#pragma region LightMethods
	/// <summary>
	/// Установка значений направленного света
	/// </summary>
	/// <param name="LightSourcePosition">Направление света, положение источника света</param>
	void SetDirectionalLight(const XMVECTOR& LightSourcePosition);
	/// <summary>
	/// Установка значений направленного света
	/// </summary>
	/// <param name="LightSourcePosition">Направление света, положение источника света</param>
	/// <param name="Color">Цвет падающего света</param>
	void SetDirectionalLight(const XMVECTOR& LightSourcePosition, const XMVECTOR& Color);
	/// <summary>
	/// Установка значений цвета света минимальной яркости мира
	/// </summary>
	/// <param name="Color">Цвет света</param>
	/// <param name="Intensity">Интенсивность света минимальной яркости мира</param>
	void SetAmbientlLight(const XMFLOAT3& Color, float Intensity);
	/// <summary>
	/// Установка значений света фонарика
	/// </summary>
	/// <param name="Color">Цвет фонарика</param>
	/// <param name="Position">Позиция фонарика</param>
	/// <param name="Direction">Направление фонарика (единичный вектор из нуля)</param>
	/// <param name="Attenuation">Угасание света</param>
	/// <param name="CutOffCosin">Угол, в пределах которого свет есть</param>
	/// <param name="OuterCutOffCosin">Угол, за пределами которого света нет</param>
	void SetSpotLight(const XMVECTOR& Color, const XMVECTOR& Position, const XMFLOAT3& Direction, const XMFLOAT3& Attenuation, float CutOffCosin, float OuterCutOffCosin);
	/// <summary>
	/// Установка значений точечного света
	/// </summary>
	/// <param name="index">Индекс источника света</param>
	/// <param name="Color">Цвет источника света</param>
	/// <param name="Position">Позиция источника света</param>
	/// <param name="Att">Угасание света</param>
	void SetPointLight(int index, const XMVECTOR& Color, const XMVECTOR& Position, const XMFLOAT3& Att);
#pragma endregion

#pragma region CameraMethods
	/// <summary>
	/// Добавляет данные о камере в массив камер
	/// </summary>
	/// <param name="CameraData">Добавляемые данные о камере</param>
	void AddCamera(const SCameraData& CameraData);
	/// <summary>
	/// Возвращает текущую камеру
	/// </summary>
	/// <returns>Текущая камера</returns>
	SCameraData* GetCurrentCamera();
	/// <summary>
	/// Устанавливает текущей камерой заданную из массива камер
	/// </summary>
	/// <param name="Index">Индекс задаваемой камеры из массива камер</param>
	void SetCamera(size_t Index);
	/// <summary>
	/// Смещает камеры
	/// </summary>
	/// <param name="Direction">Направление смещения камеры</param>
	/// <param name="StrideFactor">Величина (скорость) смещения камеры</param>
	void MoveCamera(ECameraMovementDirection Direction, float StrideFactor = 1.0f);
	/// <summary>
	/// Поворачивает камеру на заданный угол
	/// </summary>
	/// <param name="DeltaX">Угол смещения по горизонтали</param>
	/// <param name="DeltaY">Угол смещения по вертикали</param>
	/// <param name="RotationFactor">Величина (скорость) поворота камеры</param>
	void RotateCamera(int DeltaX, int DeltaY, float RotationFactor = 1.0f);
	/// <summary>
	/// Изменение дистанции между позицией и фокусом камеры
	/// </summary>
	/// <param name="DeltaWheel">Показатель изменения зума камеры</param>
	/// <param name="ZoomFactor">Величина (скорость) зума камеры</param>
	void ZoomCamera(int DeltaWheel, float ZoomFactor = 1.0f);
#pragma endregion

#pragma region ShaderMethods
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
#pragma endregion

#pragma region Object3DMethods
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
#pragma endregion

#pragma region TextureMethods
	/// <summary>
	/// Создает и возвращает указатель на текстуру
	/// </summary>
	/// <returns>Созданный указатель на текстуру</returns>
	CTexture* AddTexture();
	/// <summary>
	/// Получение указателя на текстуру с указанным индексом
	/// </summary>
	/// <param name="Index">Индекс указателя в массиве</param>
	/// <returns>Указатель на текстуру с заданным индексом</returns>
	CTexture* GetTexture(size_t Index);
#pragma endregion

#pragma region GameObjectMethods
	/// <summary>
	/// Создает и возвращает указатель на игровой обьект
	/// </summary>
	/// <returns>Созданный указатель на игровой обьект</returns>
	CGameObject* AddGameObject(const string& Name);
	/// <summary>
	/// Получение указателя на игровой обьект с заданным именем
	/// </summary>
	/// <param name="Name">Имя обьекта</param>
	/// <returns></returns>
	CGameObject* GetGameObject(const string& Name);
	/// <summary>
	/// Получение указателя на игровой обьект с указанным индексом
	/// </summary>
	/// <param name="Index">Индекс указателя в массиве</param>
	/// <returns>Указатель на игровой обьект с заданным индексом</returns>
	CGameObject* GetGameObject(size_t Index);
#pragma endregion

#pragma region RenderMethods
	/// <summary>
	/// Установка состояния растеризатора (указывает, какие грани отбраковывать)
	/// </summary>
	/// <param name="State">Новое состояние растеризатора</param>
	void SetRasterizerState(ERasterizerState State);
	/// <summary>
	/// Получает полигон/обьект, на который наведен курсор
	/// </summary>
	/// <param name="ScreenMousePositionX">Позиция курсора по X</param>
	/// <param name="ScreenMousePositionY">Позиция курсора по Y</param>
	void Pick(int ScreenMousePositionX, int ScreenMousePositionY);
	/// <summary>
	/// Обновление отрисовываемого луча picking'а
	/// </summary>
	void UpdatePickingRay();
	/// <summary>
	/// Получает название последнего выбранного обьекта
	/// </summary>
	/// <returns>Имя выбранного обьекта</returns>
	const char* GetPickedGameObjectName();
	/// <summary>
	/// Очищает буфер подкачки (задний буфер) и делает подготовку к рендерингу
	/// </summary>
	/// <param name="ClearColor">Цвет заливки области рендеринга</param>
	void BeginRendering(const FLOAT* ClearColor);
	/// <summary>
	/// Обновление анимации анимированных обьектов
	/// </summary>
	void AnimateGameObjects();
	/// <summary>
	/// Обновляет мировую матрицу всех игровых обьектов и отображает их
	/// </summary>
	/// <param name="DeltaTime">Время прошедшее между кадрами</param>
	void DrawGameObjects(float DeltaTime);
	/// <summary>
	/// Рисует представление мини осей в углу экрана
	/// </summary>
	void DrawMiniAxes();
	/// <summary>
	/// Рисует представление луча трассировки
	/// </summary>
	void DrawPickingRay();
	/// <summary>
	/// Рисует треугольник выбранный picking'ом
	/// </summary>
	void DrawPickedTriangle();
	/// <summary>
	/// Обновляет изображение, swap буфера подкачки и буфера дисплея
	/// </summary>
	void EndRendering();
#pragma endregion

public:

#pragma region GetMethods
	/// <summary>
	/// Получает дескриптор окна
	/// </summary>
	/// <returns>Дескриптор окна</returns>
	HWND GetHWND() { return m_hWnd; }
	/// <summary>
	/// Возвращает текущее состояние клавиатуры
	/// </summary>
	/// <returns>Текущее состояние клавиатуры</returns>
	Keyboard::State GetKeyState();
	/// <summary>
	/// Возвращает текущее состояние мыши и сбрасывает значение колесика прокрутки
	/// </summary>
	/// <returns>Текущее состояние мыши</returns>
	Mouse::State GetMouseState();
	/// <summary>
	/// Возвращает указатель нанабор спрайтов
	/// </summary>
	/// <returns>Указатель нанабор спрайтов</returns>
	SpriteBatch* GetSpriteBatchPtr() { return m_SpriteBatch.get(); }
	/// <summary>
	/// Возвращает указатель на спрайт шрифтов
	/// </summary>
	/// <returns>Указатель на спрайт шрифтов</returns>
	SpriteFont* GetSpriteFontPtr() { return m_SpriteFont.get(); }
#pragma endregion

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
	/// <param name="FontFileName">Путь к файлу с используемым шрифтом</param>
	/// <param name="bWindowed">Находится ли вывод в оконном режиме</param>
	void InitializeDirectX(const wstring& FontFileName, bool bWindowed);

	/// <summary>
	/// Обновление игрового обьекта
	/// </summary>
	/// <param name="PtrGO">Обновляемый обьект</param>
	/// <param name="DeltaTime">Время с последнего обновления обьекта</param>
	void UpdateGameObject(CGameObject* PtrGO, float DeltaTime);
	/// <summary>
	/// Отрисовка отдельного игрового обьекта
	/// </summary>
	/// <param name="PtrGO">Указатель на игровой обьект</param>
	void DrawGameObject(CGameObject* PtrGO);
	/// <summary>
	/// Отрисовка нормалей отдельного игрового обьекта
	/// </summary>
	/// <param name="PtrGO">Указатель на игровой обьект</param>
	void DrawGameObjectNormal(CGameObject* PtrGO);
	/// <summary>
	/// Отрисовка ограничивающей сферы
	/// </summary>
	/// <param name="PtrGO">Обьект, для которого формируется сфера</param>
	void DrawGameObjectBoundingSphere(CGameObject* PtrGO);

	/// <summary>
	/// Обновление состояния растеризатора в соответствии с заданными флагами
	/// </summary>
	void SetGameWindowCullMode();
	/// <summary>
	/// Обновление состояния освещения в шейдерах
	/// </summary>
	void SetGameWindowUseLighiting();
private:

#pragma region InitD3D11ComponentMethods
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
	/// <summary>
	/// Инициализация устройств ввода (мыши и клавиатуры)
	/// </summary>
	void CreateInputDevices();
	/// <summary>
	/// Загрузка базовых шейдеров
	/// </summary>
	void CreateShaders();
	/// <summary>
	/// Создание представления мини осей в углу экрана
	/// </summary>
	void CreateMiniAxes();
	/// <summary>
	/// Создание луча отрисовки при picking'е
	/// </summary>
	void CreatePickingRay();
	/// <summary>
	/// Создание ограничивающей сферы
	/// </summary>
	void CreateBoundingSphere();
	/// <summary>
	/// Создание отображения выбранного полигона
	/// </summary>
	void CreatePickedTriangle();
	/// <summary>
	/// Выбор ограничивающей сферы
	/// </summary>
	void PickBoundingSphere();
	/// <summary>
	/// Выбор полигона
	/// </summary>
	void PickTriangle();
#pragma endregion

private:
	static constexpr float KDefaultFOV{ XM_PIDIV2 };
	static constexpr float KDefaultNearZ{ 0.1f };
	static constexpr float KDefaultFarZ{ 1000.0f };
	static constexpr float KSkyDistance{ 30.0f };
	static constexpr float KSkyTimeFactorAbsolute{ 0.1f };
	static constexpr float KPickingRayLength{ 1000.0f };
private:
	/// <summary>
	/// Массив применяемых шейдеров
	/// </summary>
	vector<unique_ptr<CShader>>		m_vShaders{};
	/// <summary>
	/// Массив используемых 3D обьектов
	/// </summary>
	vector<unique_ptr<CObject3D>>	m_vObject3Ds{};
	/// <summary>
	/// Массив используемых текстур
	/// </summary>
	vector<unique_ptr<CTexture>>	m_vTextures{};
	/// <summary>
	/// Массив используемых игровых обьектов
	/// </summary>
	vector<unique_ptr<CGameObject>>	m_vGameObjects{};

	/// <summary>
	/// Луч трассировки Pick'инга
	/// </summary>
	unique_ptr<CObjectLine>			m_ObjectLinePickingRay{};
	/// <summary>
	/// Ограничивающая picking сфера
	/// </summary>
	unique_ptr<CObject3D>			m_Object3DBoundingSphere{};
	/// <summary>
	/// Выбранный полигон
	/// </summary>
	unique_ptr<CObject3D>			m_Object3DPickedTriangle{};

	// CGameObject* m_PtrSky{};
	// CGameObject* m_PtrCloud{};
	// CGameObject* m_PtrSun{};
	// CGameObject* m_PtrMoon{};

	/// <summary>
	/// Хешмеп хранения пары - имя обьекта - индекс обьекта
	/// </summary>
	unordered_map<string, size_t>	m_mapGameObjectNameToIndex{};

	/// <summary>
	/// Массив 3D обьектов представления мини осей в углу экрана
	/// </summary>
	vector<unique_ptr<CObject3D>>	m_vMiniAxisObject3Ds{};
	/// <summary>
	/// Массив игровых обьектов представления мини осей в углу экрана
	/// </summary>
	vector<unique_ptr<CGameObject>>	m_vMiniAxisGameObjects{};

#pragma region ShadersField

	/// <summary>
	/// Указатель на базовый вертекный шейдер
	/// </summary>
	unique_ptr<CShader>				m_VSBase{};
	/// <summary>
	/// Указатель на вертекный шейдер анимации
	/// </summary>
	unique_ptr<CShader>				m_VSAnimation{};
	
	// unique_ptr<CShader>				m_VSSky{};
	
	/// <summary>
	/// Указатель на вертексный шейдер линий
	/// </summary>
	unique_ptr<CShader>				m_VSLine{};

	/// <summary>
	/// Указатель на геометрический шейдер нормалей
	/// </summary>
	unique_ptr<CShader>				m_GSNormal{};

	/// <summary>
	/// Указатель на базовый пиксельный шейдер
	/// </summary>
	unique_ptr<CShader>				m_PSBase{};
	/// <summary>
	/// Указатель на пиксельный шейдер нормалей
	/// </summary>
	unique_ptr<CShader>				m_PSNormal{};
	
	// unique_ptr<CShader>				m_PSSky{};

	/// <summary>
	/// Указатель на пиксельный шейдер линий
	/// </summary>
	unique_ptr<CShader>				m_PSLine{};

#pragma endregion

	/// <summary>
	/// Массив используемых вьюпортов
	/// </summary>
	vector<D3D11_VIEWPORT>			m_vViewports{};
private:
	/// <summary>
	/// Идентификатор экземпляра окна
	/// </summary>
	HWND							m_hWnd{};
	/// <summary>
	/// Идентификатор экземпляра приложения
	/// </summary>
	HINSTANCE						m_hInstance{};
	/// <summary>
	/// Размер окна
	/// </summary>
	XMFLOAT2						m_WindowSize{};
private:
	/// <summary>
	/// Матрица проекции
	/// </summary>
	XMMATRIX						m_MatrixProjection{};
	/// <summary>
	/// Расстояние до ближайшей плоскости отсечения левой перспективной проекции
	/// </summary>
	float							m_NearZ{};
	/// <summary>
	/// Расстояние до дальней плоскости отсечения левой перспективной проекции
	/// </summary>
	float							m_FarZ{};
	/// <summary>
	/// Матрица вида
	/// </summary>
	XMMATRIX						m_MatrixView{};
	/// <summary>
	/// Массив камер
	/// </summary>
	vector<SCameraData>				m_vCameras{};
	/// <summary>
	/// Текущая камера
	/// </summary>
	SCameraData*					m_PtrCurrentCamera{};
	/// <summary>
	/// Вектор, направленный вперед
	/// </summary>
	XMVECTOR						m_BaseForward{};
	/// <summary>
	/// Вектор, направленный вверх
	/// </summary>
	XMVECTOR						m_BaseUp{};
private:
	XMVECTOR						m_PickingRayWorldSpaceOrigin{};//TODO непонятная фигня
	XMVECTOR						m_PickingRayWorldSpaceDirection{};//TODO непонятная фигня
	/// <summary>
	/// Указатель на выбранный игровой обьект
	/// </summary>
	CGameObject*					m_PtrPickedGameObject{};
	/// <summary>
	/// Точка выбранного треугольника 0
	/// </summary>
	XMVECTOR						m_PickedTriangleV0{};
	/// <summary>
	/// Точка выбранного треугольника 1
	/// </summary>
	XMVECTOR						m_PickedTriangleV1{};
	/// <summary>
	/// Точка выбранного треугольника 2
	/// </summary>
	XMVECTOR						m_PickedTriangleV2{};
private:
	/// <summary>
	/// Текущее состояние растеризатора (указывает, какие грани отбраковыывать)
	/// </summary>
	ERasterizerState				m_eRasterizerState{ ERasterizerState::CullCounterClockwise };
	/// <summary>
	/// Текущий флаг отрисовки
	/// </summary>
	EFlagsGameRendering				m_eFlagsGamerendering{};
private:
	/// <summary>
	/// Указатель на цепочку обмена
	/// </summary>
	ComPtr<IDXGISwapChain>			m_SwapChain{};
	/// <summary>
	/// Устройство (инструменты для создания и загрузки в видеопамять текстур, шейдеров и других компонентов)
	/// </summary>
	ComPtr<ID3D11Device>			m_Device{};
	/// <summary>
	/// Контекст устройства (инструменты для работы с отдельными этапами графического конвейера)
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
	/// <summary>
	/// Указатель на текстуру буфера глубины
	/// </summary>
	ComPtr<ID3D11Texture2D>			m_DepthStencilBuffer{};
	/// <summary>
	/// Указатель на устройство ввода клавиатура
	/// </summary>
	unique_ptr<Keyboard>			m_Keyboard{};
	/// <summary>
	/// Указатель на устройство ввода мышь
	/// </summary>
	unique_ptr<Mouse>				m_Mouse{};
	/// <summary>
	/// Указатель на набор спрайтов
	/// </summary>
	unique_ptr<SpriteBatch>			m_SpriteBatch{};
	/// <summary>
	/// Указатель на спрайт шрифта
	/// </summary>
	unique_ptr<SpriteFont>			m_SpriteFont{};
	/// <summary>
	/// Указатель на общую область рендеринга спрайтов (???)
	/// </summary>
	unique_ptr<CommonStates>		m_CommonStates{};
	
#pragma region VSBaseResources
	/// <summary>
	/// Загружаемая в вертексный шейдер структура с данными MVP и World
	/// </summary>
	SCBVSSpaceData					cbVSSpaceData{};
	/// <summary>
	/// Указатель на константный буфер с информацией об анимации
	/// </summary>
	SCBVSAnimationBonesData			cbVSAnimationBonesData{};
#pragma endregion

#pragma region PSBaseResources
	/// <summary>
	/// Загружаемая в пиксельный шейдер структура флагов
	/// </summary>
	SCBPSBaseFlagsData				cbPSBaseFlagsData{};
	/// <summary>
	/// Загружаемая в пиксельный шейдер структура освещения
	/// </summary>
	SCBPSBaseLightsData				cbPSBaseLightsData{};
	/// <summary>
	/// Загружаемая в пиксельный шейдер структура освещения фонарика
	/// </summary>
	SCBPSSpotLightsData				cbPSSpotLightsData{};
	/// <summary>
	/// Загружаемая в пиксельный шейдер структура точечного освещения 1
	/// </summary>
	SCBPSPointLightsData			cbPSPointLightsData_1{};
	/// <summary>
	/// Загружаемая в пиксельный шейдер структура точечного освещения 2
	/// </summary>
	SCBPSPointLightsData			cbPSPointLightsData_2{};
	/// <summary>
	/// Загружаемая в пиксельный шейдер структура точечного освещения 3
	/// </summary>
	SCBPSPointLightsData			cbPSPointLightsData_3{};
	/// <summary>
	/// Загружаемая в пиксельный шейдер структура точечного освещения 4
	/// </summary>
	SCBPSPointLightsData			cbPSPointLightsData_4{};
	/// <summary>
	/// Загружаемая в пиксельный шейдер структура материала
	/// </summary>
	SCBPSBaseMaterialData			cbPSBaseMaterialData{};
	/// <summary>
	/// Загружаемая в пиксельный шейдер структура позиции камеры
	/// </summary>
	SCBPSBaseEyeData				cbPSBaseEyeData{};
	/// <summary>
	/// Загружаемая в вертексный шейдер структура с данными о игровом времени
	/// </summary>
	SCBPSSkyTimeData				cbPSSkyTimeData{};
#pragma endregion
};