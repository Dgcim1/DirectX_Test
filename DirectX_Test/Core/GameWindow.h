#pragma once

#include <Windows.h>
#include "Object3D.h"
#include "Shader.h"
#include "PrimitiveGenerator.h"
#include "GameObject.h"

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
	/// <param name="FontFileName">Путь к файлу с используемым шрифтом</param>
	/// <param name="bWindowed">Находится ли вывод в оконном режиме</param>
	void CreateWin32(WNDPROC WndProc, LPCTSTR WindowName, const wstring& FontFileName, bool bWindowed);

#pragma region CameraMethods
	/// <summary>
	/// Строит матрицу левой перспективной проекции m_MatrixProjection на основе поля зрения
	/// </summary>
	/// <param name="FOV">Угол поля зрения сверху вниз в радианах</param>
	/// <param name="NearZ">Расстояние до ближайшей плоскости остечения</param>
	/// <param name="FarZ">Расстояние до дальней плоскости отсечения</param>
	void SetPerspective(float FOV, float NearZ, float FarZ);
	/// <summary>
	/// Добавляет данные о камере в массив камер
	/// </summary>
	/// <param name="CameraData">Добавляемые данные о камере</param>
	void AddCamera(const SCameraData& CameraData);
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

#pragma region GameObjectMethods
	/// <summary>
	/// Создает и возвращает указатель на игровой обьект
	/// </summary>
	/// <returns>Созданный указатель на игровой обьект</returns>
	CGameObject* AddGameObject();
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
	/// Очищает буфер подкачки (задний буфер) и делает подготовку к рендерингу
	/// </summary>
	/// <param name="ClearColor">Цвет заливки области рендеринга</param>
	void BeginRendering(const FLOAT* ClearColor);
	/// <summary>
	/// Обновляет мировую матрицу всех игровых обьектов и отображает их
	/// </summary>
	void DrawGameObjects();
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
	/// Создает CBWVP (Constant Buffer World-View-Projection) константный буфер 
	/// </summary>
	void CreateCBWVP();
#pragma endregion

	/// <summary>
	/// Обновляет константный буфер CBWVP (Constant Buffer World-View-Projection)
	/// </summary>
	/// <param name="MatrixWorld">Матрица мира</param>
	void UpdateCBWVP(const XMMATRIX& MatrixWorld);

private:
	static constexpr float KDefaultFOV{ XM_PIDIV2 };
	static constexpr float KDefaultNearZ{ 0.1f };
	static constexpr float KDefaultFarZ{ 1000.0f };
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
	/// Массив используемых игровых обьектов
	/// </summary>
	vector<unique_ptr<CGameObject>>	m_vGameObjects{};
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
	/// <summary>
	/// Матрица проекции
	/// </summary>
	XMMATRIX						m_MatrixProjection{};
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
	/// <summary>
	/// Текущее состояние растеризатора (указывает, какие грани отбраковыывать)
	/// </summary>
	ERasterizerState				m_eRasterizerState{ ERasterizerState::CullCounterClockwise };
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
	/// Указатель на константный буфер CBWVP (Constant Buffer World-View-Projection)
	/// </summary>
	ComPtr<ID3D11Buffer>			m_CBWVP{};
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
};