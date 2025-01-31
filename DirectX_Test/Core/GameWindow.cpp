#include "GameWindow.h"

//структура доступа для вертексного шейдера (элемент ввода для этапа IA)
constexpr D3D11_INPUT_ELEMENT_DESC KBaseInputElementDescs[]
{
	{ "POSITION"	, 0, DXGI_FORMAT_R32G32B32A32_FLOAT	, 0,  0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "COLOR"		, 0, DXGI_FORMAT_R32G32B32A32_FLOAT	, 0, 16, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "TEXCOORD"	, 0, DXGI_FORMAT_R32G32B32A32_FLOAT	, 0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "NORMAL"		, 0, DXGI_FORMAT_R32G32B32A32_FLOAT	, 0, 48, D3D11_INPUT_PER_VERTEX_DATA, 0 },

	{ "BLENDINDICES", 0, DXGI_FORMAT_R32G32B32A32_UINT	, 1,  0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "BLENDWEIGHT"	, 0, DXGI_FORMAT_R32G32B32A32_FLOAT	, 1, 16, D3D11_INPUT_PER_VERTEX_DATA, 0 },
};
//структура для вертексного шейдера линий
constexpr D3D11_INPUT_ELEMENT_DESC KVSLineInputElementDescs[]
{
	{ "POSITION"	, 0, DXGI_FORMAT_R32G32B32A32_FLOAT	, 0,  0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "COLOR"		, 0, DXGI_FORMAT_R32G32B32A32_FLOAT	, 0, 16, D3D11_INPUT_PER_VERTEX_DATA, 0 },
};

void CGameWindow::CreateWin32(WNDPROC WndProc, LPCTSTR WindowName, const wstring& FontFileName, bool bWindowed)
{
	CreateWin32Window(WndProc, WindowName);

	InitializeDirectX(FontFileName, bWindowed);
}

void CGameWindow::SetPerspective(float FOV, float NearZ, float FarZ)
{
	m_NearZ = NearZ;
	m_FarZ = FarZ;

	m_MatrixProjection = XMMatrixPerspectiveFovLH(FOV, m_WindowSize.x / m_WindowSize.y, m_NearZ, m_FarZ);
}

void CGameWindow::SetGameRenderingFlags(EFlagsGameRendering Flags)
{
	m_eFlagsGamerendering = Flags;
}

void CGameWindow::ToggleGameRenderingFlags(EFlagsGameRendering Flags)
{
	m_eFlagsGamerendering ^= Flags;
}

void CGameWindow::SetDirectionalLight(const XMVECTOR& LightSourcePosition)
{
	cbPSBaseLightsData.DirectionalLightDirection = XMVector3Normalize(LightSourcePosition);

	m_PSBase->UpdateConstantBuffer(1);
}

void CGameWindow::SetDirectionalLight(const XMVECTOR& LightSourcePosition, const XMVECTOR& Color)
{
	cbPSBaseLightsData.DirectionalLightDirection = XMVector3Normalize(LightSourcePosition);
	cbPSBaseLightsData.DirectionalColor = Color;

	m_PSBase->UpdateConstantBuffer(1);
}

void CGameWindow::SetAmbientlLight(const XMFLOAT3& Color, float Intensity)
{
	cbPSBaseLightsData.AmbientLightColor = Color;
	cbPSBaseLightsData.AmbientLightIntensity = Intensity;

	m_PSBase->UpdateConstantBuffer(1);
}

void CGameWindow::SetSpotLight(const XMVECTOR& Color, const XMVECTOR& Position, const XMFLOAT3& Direction, const XMFLOAT3& Attenuation, float CutOffCosin, float OuterCutOffCosin)
{
	cbPSSpotLightsData.SpotlightColor = Color;
	cbPSSpotLightsData.SpotlightPosition = Position;
	cbPSSpotLightsData.SpotlightDirection = Direction;
	cbPSSpotLightsData.SpotlightAttenuation = Attenuation;
	cbPSSpotLightsData.SpotlightCutOffCosin = CutOffCosin;
	cbPSSpotLightsData.SpotlightOuterCutOffCosin = OuterCutOffCosin;

	m_PSBase->UpdateConstantBuffer(4);
}

void CGameWindow::SetPointLight(int index, const XMVECTOR& Color, const XMVECTOR& Position, const XMFLOAT3& Att)
{
	switch (index)
	{
	case 0:
		cbPSPointLightsData_1.PointlightColor = Color;
		cbPSPointLightsData_1.PointlightPosition = Position;
		cbPSPointLightsData_1.PointlightAttenuation = Att;
		m_PSBase->UpdateConstantBuffer(5);
		break;
	case 1:
		cbPSPointLightsData_2.PointlightColor = Color;
		cbPSPointLightsData_2.PointlightPosition = Position;
		cbPSPointLightsData_2.PointlightAttenuation = Att;
		m_PSBase->UpdateConstantBuffer(6);
		break;
	case 2:
		cbPSPointLightsData_3.PointlightColor = Color;
		cbPSPointLightsData_3.PointlightPosition = Position;
		cbPSPointLightsData_3.PointlightAttenuation = Att;
		m_PSBase->UpdateConstantBuffer(7);
		break;
	case 3:
		cbPSPointLightsData_4.PointlightColor = Color;
		cbPSPointLightsData_4.PointlightPosition = Position;
		cbPSPointLightsData_4.PointlightAttenuation = Att;
		m_PSBase->UpdateConstantBuffer(8);
		break;
	default:
		break;
	}
}

void CGameWindow::AddCamera(const SCameraData& CameraData)
{
	m_vCameras.emplace_back(CameraData);
}

SCameraData* CGameWindow::GetCurrentCamera() {
	return m_PtrCurrentCamera;
}

void CGameWindow::SetCamera(size_t Index)
{
	assert(Index < m_vCameras.size());

	m_PtrCurrentCamera = &m_vCameras[Index];

	m_BaseForward = m_PtrCurrentCamera->Forward = XMVector3Normalize(m_PtrCurrentCamera->FocusPosition - m_PtrCurrentCamera->EyePosition);
	m_BaseUp = m_PtrCurrentCamera->UpDirection;

	if (m_PtrCurrentCamera->CameraType == ECameraType::ThirdPerson)
	{
		m_PtrCurrentCamera->EyePosition = m_PtrCurrentCamera->FocusPosition - m_BaseForward * m_PtrCurrentCamera->Distance;
	}
}

void CGameWindow::MoveCamera(ECameraMovementDirection Direction, float StrideFactor)
{
	assert(m_PtrCurrentCamera);

	XMVECTOR dPosition{};//отклонение от текущей позиции
	//если тип камеры - свободная
	if (m_PtrCurrentCamera->CameraType == ECameraType::FreeLook)
	{
		XMVECTOR Rightward{ XMVector3Normalize(XMVector3Cross(m_PtrCurrentCamera->UpDirection, m_PtrCurrentCamera->Forward)) };//нормальный вектор направленный вправо (нормализованное перекрестное произведение векторов направления вверх относительно камеры и вперед)

		switch (Direction)
		{
		case ECameraMovementDirection::Forward:
			dPosition = +m_PtrCurrentCamera->Forward * StrideFactor;
			break;
		case ECameraMovementDirection::Backward:
			dPosition = -m_PtrCurrentCamera->Forward * StrideFactor;
			break;
		case ECameraMovementDirection::Rightward:
			dPosition = +Rightward * StrideFactor;
			break;
		case ECameraMovementDirection::Leftward:
			dPosition = -Rightward * StrideFactor;
			break;
		default:
			break;
		}
	}
	//если тип камеры от первого/третьего лица
	else if (m_PtrCurrentCamera->CameraType == ECameraType::FirstPerson || m_PtrCurrentCamera->CameraType == ECameraType::ThirdPerson)
	{
		XMVECTOR GroundRightward{ XMVector3Normalize(XMVector3Cross(m_BaseUp, m_PtrCurrentCamera->Forward)) };//нормальный вектор направленный вправо относительно сцены (нормализованное перекрестное произведение векторов направления вверх и вперед)
		XMVECTOR GroundForward{ XMVector3Normalize(XMVector3Cross(GroundRightward, m_BaseUp)) };//Вектор вперед относительно сцены

		switch (Direction)
		{
		case ECameraMovementDirection::Forward:
			dPosition = +GroundForward * StrideFactor;
			break;
		case ECameraMovementDirection::Backward:
			dPosition = -GroundForward * StrideFactor;
			break;
		case ECameraMovementDirection::Rightward:
			dPosition = +GroundRightward * StrideFactor;
			break;
		case ECameraMovementDirection::Leftward:
			dPosition = -GroundRightward * StrideFactor;
			break;
		default:
			break;
		}
	}
	if (m_PtrCurrentCamera->isCollision) {
		BoundingBox collisionBox;
		XMFLOAT3 cameraPos;
		XMStoreFloat3(&cameraPos, m_PtrCurrentCamera->EyePosition + dPosition);
		collisionBox.Center = cameraPos;
		collisionBox.Extents = m_PtrCurrentCamera->Extents;
		for (auto& gameObject : m_vGameObjects) {
			if (gameObject->ComponentPhysics.isCollide) {
				if (gameObject->ComponentPhysics.collisionBox.Contains(collisionBox) != DISJOINT)
					return;
			}
		}
	}
	m_PtrCurrentCamera->EyePosition += dPosition;
	m_PtrCurrentCamera->FocusPosition += dPosition;
}

void CGameWindow::RotateCamera(int DeltaX, int DeltaY, float RotationFactor)
{
	assert(m_PtrCurrentCamera);

	m_PtrCurrentCamera->Pitch += RotationFactor * DeltaY;
	m_PtrCurrentCamera->Yaw += RotationFactor * DeltaX;

	static constexpr float KPitchLimit{ XM_PIDIV2 - 0.01f };//угол отклонения камеры по вертикали
	m_PtrCurrentCamera->Pitch = max(-KPitchLimit, m_PtrCurrentCamera->Pitch);
	m_PtrCurrentCamera->Pitch = min(+KPitchLimit, m_PtrCurrentCamera->Pitch);

	if (m_PtrCurrentCamera->Yaw > XM_PI)
	{
		m_PtrCurrentCamera->Yaw -= XM_2PI;
	}
	else if (m_PtrCurrentCamera->Yaw < -XM_PI)
	{
		m_PtrCurrentCamera->Yaw += XM_2PI;
	}
	//определяем, где теперь вперед и вверх для камеры
	XMMATRIX MatrixRotation{ XMMatrixRotationRollPitchYaw(m_PtrCurrentCamera->Pitch, m_PtrCurrentCamera->Yaw, 0) };//матрица вращения (на основе углов поворота по осям X Y Z соответственно)
	m_PtrCurrentCamera->Forward = XMVector3TransformNormal(m_BaseForward, MatrixRotation);
	XMVECTOR Rightward{ XMVector3Normalize(XMVector3Cross(m_BaseUp, m_PtrCurrentCamera->Forward)) };//вектор, определяющий право для камеры
	XMVECTOR Upward{ XMVector3Normalize(XMVector3Cross(m_PtrCurrentCamera->Forward, Rightward)) };//вектор, определяющий верх для камеры

	m_PtrCurrentCamera->UpDirection = Upward;
	//если камера от первого лица или свободная, то меняем фокус (направление взгляда) камеры
	if (m_PtrCurrentCamera->CameraType == ECameraType::FirstPerson || m_PtrCurrentCamera->CameraType == ECameraType::FreeLook)
	{
		m_PtrCurrentCamera->FocusPosition = m_PtrCurrentCamera->EyePosition + m_PtrCurrentCamera->Forward;
	}
	//если камера от третьего лица, то меняем позицию камеры
	else if (m_PtrCurrentCamera->CameraType == ECameraType::ThirdPerson)
	{
		m_PtrCurrentCamera->EyePosition = m_PtrCurrentCamera->FocusPosition - m_PtrCurrentCamera->Forward * m_PtrCurrentCamera->Distance;
	}
}

void CGameWindow::ZoomCamera(int DeltaWheel, float ZoomFactor)
{
	m_PtrCurrentCamera->Distance -= DeltaWheel * ZoomFactor;
	m_PtrCurrentCamera->Distance = max(m_PtrCurrentCamera->Distance, m_PtrCurrentCamera->MinDistance);
	m_PtrCurrentCamera->Distance = min(m_PtrCurrentCamera->Distance, m_PtrCurrentCamera->MaxDistance);

	m_PtrCurrentCamera->EyePosition = m_PtrCurrentCamera->FocusPosition - m_PtrCurrentCamera->Forward * m_PtrCurrentCamera->Distance;
}

void CGameWindow::CreateWin32Window(WNDPROC WndProc, LPCTSTR WindowName)
{
	assert(!m_hWnd);

	constexpr LPCTSTR KClassName{ TEXT("GameWindow") };
	constexpr DWORD KWindowStyle{ WS_CAPTION | WS_SYSMENU };

	//создаем и регистрируем класс окна
	WNDCLASSEX WindowClass{};
	WindowClass.cbSize = sizeof(WNDCLASSEX);
	WindowClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	WindowClass.hCursor = LoadCursor(nullptr, IDC_ARROW);
	WindowClass.hIcon = WindowClass.hIconSm = LoadIcon(nullptr, IDI_APPLICATION);
	WindowClass.hInstance = m_hInstance;
	WindowClass.lpfnWndProc = WndProc;
	WindowClass.lpszClassName = KClassName;
	WindowClass.lpszMenuName = nullptr;
	WindowClass.style = CS_VREDRAW | CS_HREDRAW;
	RegisterClassEx(&WindowClass);

	RECT WindowRect{};
	WindowRect.right = static_cast<LONG>(m_WindowSize.x);
	WindowRect.bottom = static_cast<LONG>(m_WindowSize.y);
	AdjustWindowRect(&WindowRect, KWindowStyle, false);

	m_hWnd = CreateWindowEx(0, KClassName, WindowName, KWindowStyle,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		WindowRect.right - WindowRect.left,
		WindowRect.bottom - WindowRect.top,
		nullptr, nullptr, m_hInstance, nullptr);

	ShowWindow(m_hWnd, SW_SHOW);

	UpdateWindow(m_hWnd);

	assert(m_hWnd);
}

void CGameWindow::InitializeDirectX(const wstring& FontFileName, bool bWindowed)
{
	CreateSwapChain(bWindowed);

	CreateSetViews();

	SetViewports();

	SetPerspective(KDefaultFOV, KDefaultNearZ, KDefaultFarZ);

	CreateInputDevices();

	CreateShaders();

	CreateMiniAxes();

	CreatePickingRay();
	CreatePickedTriangle();

	CreateBoundingSphere();

	m_SpriteBatch = make_unique<SpriteBatch>(m_DeviceContext.Get());
	m_SpriteFont = make_unique<SpriteFont>(m_Device.Get(), FontFileName.c_str());
	m_CommonStates = make_unique<CommonStates>(m_Device.Get());

	m_2dFrame3d = new CObject3D(m_Device.Get(), m_DeviceContext.Get(), this);
	{
		m_2dFrame3d->Create(GenerateSquareXYPlane());
	}
	m_2dFrame = new CGameObject("_gameFrame");
	{
		m_2dFrame->ComponentTransform.Translation = XMVectorSet(0.0f, 0.0f, 0.0f, 0);
		m_2dFrame->ComponentTransform.Translation = XMVectorSet(0.0f, 0.0f, 0.0f, 0);
		m_2dFrame->UpdateWorldMatrix();
		m_2dFrame->ComponentRender.PtrObject3D = m_2dFrame3d;
		m_2dFrame->ComponentRender.IsTransparent = false;
		m_2dFrame->ComponentPhysics.bIsPickable = false;
		m_2dFrame->ComponentPhysics.BoundingSphere.CenterOffset = XMVectorSet(0.0f, 1.6f, 0.0f, 0);
		m_2dFrame->ComponentPhysics.BoundingSphere.Radius = 1.8f;
	}

	D3D11_TEXTURE2D_DESC desc{};//создаем трафаретную поверхность глубины
	desc.ArraySize = 1;//количество текстур в массиве текстур
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;//описывает как привязать данную текстуру к конвейеру (сейчас как буфер (трафарет) глубины)
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;//тип доступа к ЦП (0- доступ к ЦП не требуется) (тут может быть 0)
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;//формат буфера (сейчас 24 бита глубины и 8 бит трафарета)
	desc.Width = static_cast<UINT>(m_WindowSize.x);//ширина текстуры
	desc.Height = static_cast<UINT>(m_WindowSize.y);//высота текстуры
	desc.MipLevels = 1;//максимальное количество уровней MIP-карты текстуры (0 для отображения без мультисемплинга)
	desc.MiscFlags = 0;//флаги
	desc.SampleDesc.Count = 1;//количество мультисэмплов на пиксель
	desc.SampleDesc.Quality = 0;//уровень качества изображения
	desc.Usage = D3D11_USAGE_DEFAULT;//способ чтения и записи текстуры (сейчас все со стороны GPU) (D3D11_USAGE_DYNAMIC все ломает)
	m_Device->CreateTexture2D(&desc, nullptr, &pTextureOther1);
	m_Device->CreateTexture2D(&desc, nullptr, &pTextureOther2);
	m_Device->CreateTexture2D(&desc, nullptr, &pTextureRenderTarget);
	m_Device->CreateRenderTargetView(pTextureRenderTarget, nullptr, &m_OtherRenderTargetView);//создаем представление для доступа к данным
}

void CGameWindow::CreateSwapChain(bool bWindowed)
{
	DXGI_SWAP_CHAIN_DESC SwapChainDesc{};//структура, описывающая свойства переднего буфера
	SwapChainDesc.BufferCount = 1;//Количество буферов в очереди
	//режим отображения
	SwapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;//формат отображения (формат пикселя буфера)
	SwapChainDesc.BufferDesc.Width = static_cast<UINT>(m_WindowSize.x);//ширина разрешения (ширина буфера)
	SwapChainDesc.BufferDesc.Height = static_cast<UINT>(m_WindowSize.y);//высота разрешения (высота буфера)
	//частота обновления в герцах (представлено как 60/1=60)
	SwapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	SwapChainDesc.BufferDesc.RefreshRate.Numerator = 60;//частота обновления экрана
	SwapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;//режим масштабирования (как изображение растягивается при изменении масштаба для соответствия разрешению монитора)
	SwapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;//в каком порядке растеризатор создает изображение на поверхности (сейчас порядок не указан)
	SwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;//как будет использоваться поверхность/ресурс и параметры доступа к ним со стороны ЦП для заднего буфера (сейчас поверхность/ресурс - выходная цель рендеринга) (вкратце - назначение буфера)
	SwapChainDesc.Flags = 0;//параметры поведения цепочки (очереди) подкачки
	SwapChainDesc.OutputWindow = m_hWnd;//дескриптор окна вывода (к какому окну привязан буфер)
	SwapChainDesc.SampleDesc.Count = 1;//количество мультисэмплов на пиксель
	SwapChainDesc.SampleDesc.Quality = 0;//уровень качества изображения
	SwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;//параметры обработки пикселей на поверхности отображения после вызова Present1() (как то связано с мультисэмплингом)
	SwapChainDesc.Windowed = bWindowed;//true - оконный / false - полноэкранный режим
	//создаем устройство - адаптер дисплея и цепочку подкачки
	D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, D3D10_CREATE_DEVICE_FLAG::D3D10_CREATE_DEVICE_DEBUG, nullptr, 0, D3D11_SDK_VERSION,
		&SwapChainDesc, &m_SwapChain, &m_Device, nullptr, &m_DeviceContext);
}

void CGameWindow::CreateSetViews()
{
	ComPtr<ID3D11Texture2D> BackBuffer{};
	m_SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), &BackBuffer);//получаем указатель на 1-й задний буфер подкачки

	m_Device->CreateRenderTargetView(BackBuffer.Get(), nullptr, &m_RenderTargetView);//создаем представление для доступа к данным

	D3D11_TEXTURE2D_DESC DepthStencilBufferDesc{};//создаем трафаретную поверхность глубины
	DepthStencilBufferDesc.ArraySize = 1;//количество текстур в массиве текстур
	DepthStencilBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;//описывает как привязать данную текстуру к конвейеру (сейчас как буфер (трафарет) глубины)
	DepthStencilBufferDesc.CPUAccessFlags = 0;//тип доступа к ЦП (0- доступ к ЦП не требуется)
	DepthStencilBufferDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;//формат буфера (сейчас 24 бита глубины и 8 бит трафарета)
	DepthStencilBufferDesc.Width = static_cast<UINT>(m_WindowSize.x);//ширина текстуры
	DepthStencilBufferDesc.Height = static_cast<UINT>(m_WindowSize.y);//высота текстуры
	DepthStencilBufferDesc.MipLevels = 0;//максимальное количество уровней MIP-карты текстуры (0 для отображения без мультисемплинга)
	DepthStencilBufferDesc.MiscFlags = 0;//флаги
	DepthStencilBufferDesc.SampleDesc.Count = 1;//количество мультисэмплов на пиксель
	DepthStencilBufferDesc.SampleDesc.Quality = 0;//уровень качества изображения
	DepthStencilBufferDesc.Usage = D3D11_USAGE_DEFAULT;//способ чтения и записи текстуры (сейчас все со стороны GPU)
	m_Device->CreateTexture2D(&DepthStencilBufferDesc, nullptr, &m_DepthStencilBuffer);
	m_Device->CreateDepthStencilView(m_DepthStencilBuffer.Get(), nullptr, &m_DepthStencilView);//создаем представление для буфера глубины
	//ОМ - этап слияния вывода, последний этап для определения видимых пикселей
	m_DeviceContext->OMSetRenderTargets(1, m_RenderTargetView.GetAddressOf(), m_DepthStencilView.Get());//привязка цели рендеринга (заднего буфера) и трафарета глубины к этапу ОМ
}

void CGameWindow::SetViewports()
{
	//основной вьюпорт
	{
		m_vViewports.emplace_back();

		D3D11_VIEWPORT& ViewPort{ m_vViewports.back() };
		ViewPort.TopLeftX = 0.0f;//положение Х для крайней левой точки
		ViewPort.TopLeftY = 0.0f;//положение Y для крайней верхней точки
		ViewPort.Width = m_WindowSize.x;//ширина области просмотра
		ViewPort.Height = m_WindowSize.y;//высота области просмотра
		ViewPort.MinDepth = 0.0f;//минимальная глубина области просмотра
		ViewPort.MaxDepth = 1.0f;//максимальная глубина области просмотра
	}

	//дополнительный вьюпорт для отрисовки представления мини осей в углу экрана
	{
		m_vViewports.emplace_back();

		D3D11_VIEWPORT& Viewport{ m_vViewports.back() };
		Viewport.TopLeftX = m_WindowSize.x * 0.875f;
		Viewport.TopLeftY = 0.0f;
		Viewport.Width = m_WindowSize.x / 8.0f;
		Viewport.Height = m_WindowSize.y / 8.0f;
		Viewport.MinDepth = 0.0f;
		Viewport.MaxDepth = 1.0f;
	}
}

void CGameWindow::CreateInputDevices()
{
	m_Keyboard = make_unique<Keyboard>();

	m_Mouse = make_unique<Mouse>();
	m_Mouse->SetWindow(m_hWnd);

	m_Mouse->SetMode(Mouse::Mode::MODE_ABSOLUTE);
	m_Mouse->SetVisible(false);
}

void CGameWindow::CreateShaders()
{
	m_VSBase = make_unique<CShader>(m_Device.Get(), m_DeviceContext.Get());
	m_VSBase->Create(EShaderType::VertexShader, L"Shader\\VSBase.hlsl", "main", KBaseInputElementDescs, ARRAYSIZE(KBaseInputElementDescs));
	m_VSBase->AddConstantBuffer(&cbVSSpaceData, sizeof(SCBVSSpaceData));

	// m_VSAnimation = make_unique<CShader>(m_Device.Get(), m_DeviceContext.Get());
	// m_VSAnimation->Create(EShaderType::VertexShader, L"Shader\\VSAnimation.hlsl", "main", KBaseInputElementDescs, ARRAYSIZE(KBaseInputElementDescs));
	// m_VSAnimation->AddConstantBuffer(&cbVSSpaceData, sizeof(SCBVSSpaceData));
	// m_VSAnimation->AddConstantBuffer(&cbVSAnimationBonesData, sizeof(SCBVSAnimationBonesData));
	// 
	// m_VSSky = make_unique<CShader>(m_Device.Get(), m_DeviceContext.Get());
	// m_VSSky->Create(EShaderType::VertexShader, L"Shader\\VSSky.hlsl", "main", KBaseInputElementDescs, ARRAYSIZE(KBaseInputElementDescs));
	// m_VSSky->AddConstantBuffer(&cbVSSpaceData, sizeof(SCBVSSpaceData));

	m_VSLine = make_unique<CShader>(m_Device.Get(), m_DeviceContext.Get());
	m_VSLine->Create(EShaderType::VertexShader, L"Shader\\VSLine.hlsl", "main", KVSLineInputElementDescs, ARRAYSIZE(KVSLineInputElementDescs));
	m_VSLine->AddConstantBuffer(&cbVSSpaceData, sizeof(SCBVSSpaceData));

	m_GSNormal = make_unique<CShader>(m_Device.Get(), m_DeviceContext.Get());
	m_GSNormal->Create(EShaderType::GeometryShader, L"Shader\\GSNormal.hlsl", "main");

	m_PSBase = make_unique<CShader>(m_Device.Get(), m_DeviceContext.Get());
	m_PSBase->Create(EShaderType::PixelShader, L"Shader\\PSBase.hlsl", "main");
	m_PSBase->AddConstantBuffer(&cbPSBaseFlagsData, sizeof(SCBPSBaseFlagsData));
	m_PSBase->AddConstantBuffer(&cbPSBaseLightsData, sizeof(SCBPSBaseLightsData));
	m_PSBase->AddConstantBuffer(&cbPSBaseMaterialData, sizeof(SCBPSBaseMaterialData));
	m_PSBase->AddConstantBuffer(&cbPSBaseEyeData, sizeof(SCBPSBaseEyeData));
	m_PSBase->AddConstantBuffer(&cbPSSpotLightsData, sizeof(SCBPSSpotLightsData));
	m_PSBase->AddConstantBuffer(&cbPSPointLightsData_1, sizeof(SCBPSPointLightsData));
	m_PSBase->AddConstantBuffer(&cbPSPointLightsData_2, sizeof(SCBPSPointLightsData));
	m_PSBase->AddConstantBuffer(&cbPSPointLightsData_3, sizeof(SCBPSPointLightsData));
	m_PSBase->AddConstantBuffer(&cbPSPointLightsData_4, sizeof(SCBPSPointLightsData));

	m_PSNormal = make_unique<CShader>(m_Device.Get(), m_DeviceContext.Get());
	m_PSNormal->Create(EShaderType::PixelShader, L"Shader\\PSNormal.hlsl", "main");

	// m_PSSky = make_unique<CShader>(m_Device.Get(), m_DeviceContext.Get());
	// m_PSSky->Create(EShaderType::PixelShader, L"Shader\\PSSky.hlsl", "main");
	// m_PSSky->AddConstantBuffer(&cbPSSkyTimeData, sizeof(SCBPSSkyTimeData));

	m_PSLine = make_unique<CShader>(m_Device.Get(), m_DeviceContext.Get());
	m_PSLine->Create(EShaderType::PixelShader, L"Shader\\PSLine.hlsl", "main");

	m_PSOutlineGlowing = make_unique<CShader>(m_Device.Get(), m_DeviceContext.Get());
	m_PSOutlineGlowing->Create(EShaderType::PixelShader, L"Shader\\PSOutlineGlowing.hlsl", "main");

	m_PSStencil = make_unique<CShader>(m_Device.Get(), m_DeviceContext.Get());
	m_PSStencil->Create(EShaderType::PixelShader, L"Shader\\PSStencil.hlsl", "main");
}

void CGameWindow::CreateMiniAxes()
{
	m_vMiniAxisObject3Ds.emplace_back(make_unique<CObject3D>(m_Device.Get(), m_DeviceContext.Get(), this));
	m_vMiniAxisObject3Ds.emplace_back(make_unique<CObject3D>(m_Device.Get(), m_DeviceContext.Get(), this));
	m_vMiniAxisObject3Ds.emplace_back(make_unique<CObject3D>(m_Device.Get(), m_DeviceContext.Get(), this));

	SMesh Cone{ GenerateCone(0, 16) };
	vector<SMaterial> vMaterials{ SMaterial(XMFLOAT3(1, 0, 0)), SMaterial(XMFLOAT3(0, 1, 0)), SMaterial(XMFLOAT3(0, 0, 1)) };
	m_vMiniAxisObject3Ds[0]->Create(Cone, vMaterials[0]);
	m_vMiniAxisObject3Ds[1]->Create(Cone, vMaterials[1]);
	m_vMiniAxisObject3Ds[2]->Create(Cone, vMaterials[2]);

	m_vMiniAxisGameObjects.emplace_back(make_unique<CGameObject>("AxisX"));
	m_vMiniAxisGameObjects.emplace_back(make_unique<CGameObject>("AxisY"));
	m_vMiniAxisGameObjects.emplace_back(make_unique<CGameObject>("AxisZ"));
	m_vMiniAxisGameObjects[0]->ComponentRender.PtrObject3D = m_vMiniAxisObject3Ds[0].get();
	m_vMiniAxisGameObjects[0]->ComponentTransform.RotationQuaternion = XMQuaternionRotationRollPitchYaw(0, 0, -XM_PIDIV2);
	m_vMiniAxisGameObjects[1]->ComponentRender.PtrObject3D = m_vMiniAxisObject3Ds[1].get();
	m_vMiniAxisGameObjects[2]->ComponentRender.PtrObject3D = m_vMiniAxisObject3Ds[2].get();
	m_vMiniAxisGameObjects[2]->ComponentTransform.RotationQuaternion = XMQuaternionRotationRollPitchYaw(0, -XM_PIDIV2, -XM_PIDIV2);

	m_vMiniAxisGameObjects[0]->ComponentTransform.Scaling =
		m_vMiniAxisGameObjects[1]->ComponentTransform.Scaling =
		m_vMiniAxisGameObjects[2]->ComponentTransform.Scaling = XMVectorSet(0.1f, 1.0f, 0.1f, 0);

	m_vMiniAxisGameObjects[0]->UpdateWorldMatrix();
	m_vMiniAxisGameObjects[1]->UpdateWorldMatrix();
	m_vMiniAxisGameObjects[2]->UpdateWorldMatrix();
}

void CGameWindow::CreatePickingRay()
{
	m_ObjectLinePickingRay = make_unique<CObjectLine>(m_Device.Get(), m_DeviceContext.Get());

	vector<SVertexLine> Vertices{};
	Vertices.emplace_back(XMVectorSet(0, 0, 0, 1), XMVectorSet(1, 0, 0, 1));
	Vertices.emplace_back(XMVectorSet(10.0f, 10.0f, 0, 1), XMVectorSet(0, 1, 0, 1));

	m_ObjectLinePickingRay->Create(Vertices);
}

void CGameWindow::CreateBoundingSphere()
{
	m_Object3DBoundingSphere = make_unique<CObject3D>(m_Device.Get(), m_DeviceContext.Get(), this);

	m_Object3DBoundingSphere->Create(GenerateSphere(16));
}

void CGameWindow::CreatePickedTriangle()
{
	m_Object3DPickedTriangle = make_unique<CObject3D>(m_Device.Get(), m_DeviceContext.Get(), this);

	m_Object3DPickedTriangle->Create(GenerateTriangle(XMVectorSet(0, 0, 1.5f, 1), XMVectorSet(+1.0f, 0, 0, 1), XMVectorSet(-1.0f, 0, 0, 1),
		XMVectorSet(1.0f, 1.0f, 0.0f, 1.0f)));
}

void CGameWindow::PickBoundingSphere()
{
	m_PtrPickedGameObject = nullptr;

	XMVECTOR T{ KVectorGreatest };
	for (auto& i : m_vGameObjects)
	{
		auto* GO{ i.get() };
		if (GO->ComponentPhysics.bIsPickable)
		{
			XMVECTOR NewT{ KVectorGreatest };
			if (IntersectRaySphere(m_PickingRayWorldSpaceOrigin, m_PickingRayWorldSpaceDirection,
				GO->ComponentPhysics.BoundingSphere.Radius, GO->ComponentTransform.Translation + GO->ComponentPhysics.BoundingSphere.CenterOffset, &NewT))
			{
				if (XMVector3Less(NewT, T))
				{
					T = NewT;
					m_PtrPickedGameObject = GO;
				}
			}
		}
	}
}

void CGameWindow::PickTriangle()
{
	XMVECTOR T{ KVectorGreatest };
	if (m_PtrPickedGameObject)
	{
		assert(m_PtrPickedGameObject->ComponentRender.PtrObject3D);

		// Pick only static models' triangle.
		if (m_PtrPickedGameObject->ComponentRender.PtrObject3D->m_Model.bIsAnimated) return;

		const XMMATRIX& World{ m_PtrPickedGameObject->ComponentTransform.MatrixWorld };
		for (auto& Mesh : m_PtrPickedGameObject->ComponentRender.PtrObject3D->m_Model.vMeshes)
		{
			for (auto& Triangle : Mesh.vTriangles)
			{
				XMVECTOR V0{ Mesh.vVertices[Triangle.I0].Position };
				XMVECTOR V1{ Mesh.vVertices[Triangle.I1].Position };
				XMVECTOR V2{ Mesh.vVertices[Triangle.I2].Position };
				V0 = XMVector3TransformCoord(V0, World);
				V1 = XMVector3TransformCoord(V1, World);
				V2 = XMVector3TransformCoord(V2, World);

				XMVECTOR NewT{};
				if (IntersectRayTriangle(m_PickingRayWorldSpaceOrigin, m_PickingRayWorldSpaceDirection, V0, V1, V2, &NewT))
				{
					if (XMVector3Less(NewT, T))
					{
						T = NewT;

						XMVECTOR N{ CalculateTriangleNormal(V0, V1, V2) };

						m_PickedTriangleV0 = V0 + N * 0.01f;
						m_PickedTriangleV1 = V1 + N * 0.01f;
						m_PickedTriangleV2 = V2 + N * 0.01f;
					}
				}
			}
		}
	}
}

CShader* CGameWindow::AddShader()
{
	m_vShaders.emplace_back(make_unique<CShader>(m_Device.Get(), m_DeviceContext.Get()));
	return m_vShaders.back().get();
}

CShader* CGameWindow::GetShader(size_t Index)
{
	assert(Index < m_vShaders.size());
	return m_vShaders[Index].get();
}

CObject3D* CGameWindow::AddObject3D()
{
	m_vObject3Ds.emplace_back(make_unique<CObject3D>(m_Device.Get(), m_DeviceContext.Get(), this));
	return m_vObject3Ds.back().get();
}

CObject3D* CGameWindow::GetObject3D(size_t Index)
{
	assert(Index < m_vObject3Ds.size());
	return m_vObject3Ds[Index].get();
}

CTexture* CGameWindow::AddTexture()
{
	m_vTextures.emplace_back(make_unique<CTexture>(m_Device.Get(), m_DeviceContext.Get()));
	return m_vTextures.back().get();
}

CTexture* CGameWindow::GetTexture(size_t Index)
{
	assert(Index < m_vTextures.size());
	return m_vTextures[Index].get();
}

CGameObject* CGameWindow::AddGameObject(const string& Name)
{
	assert(m_mapGameObjectNameToIndex.find(Name) == m_mapGameObjectNameToIndex.end());

	m_vGameObjects.emplace_back(make_unique<CGameObject>(Name));

	m_mapGameObjectNameToIndex[Name] = m_vGameObjects.size() - 1;

	return m_vGameObjects.back().get();
}

CGameObject* CGameWindow::GetGameObject(const string& Name)
{
	assert(m_mapGameObjectNameToIndex.find(Name) != m_mapGameObjectNameToIndex.end());
	return m_vGameObjects[m_mapGameObjectNameToIndex[Name]].get();
}

CGameObject* CGameWindow::GetGameObject(size_t Index)
{
	assert(Index < m_vGameObjects.size());
	return m_vGameObjects[Index].get();
}

void CGameWindow::SetRasterizerState(ERasterizerState State)
{
	m_eRasterizerState = State;
}

void CGameWindow::Pick(int ScreenMousePositionX, int ScreenMousePositionY)
{
	float ViewSpaceRayDirectionX{ (ScreenMousePositionX / (m_WindowSize.x / 2.0f) - 1.0f) / XMVectorGetX(m_MatrixProjection.r[0]) };
	float ViewSpaceRayDirectionY{ (-(ScreenMousePositionY / (m_WindowSize.y / 2.0f) - 1.0f)) / XMVectorGetY(m_MatrixProjection.r[1]) };
	float ViewSpaceRayDirectionZ{ 1.0f };

	XMVECTOR ViewSpaceRayOrigin{ XMVectorSet(0, 0, 0, 1) };
	XMVECTOR ViewSpaceRayDirection{ XMVectorSet(ViewSpaceRayDirectionX, ViewSpaceRayDirectionY, ViewSpaceRayDirectionZ, 0) };

	XMMATRIX MatrixViewInverse{ XMMatrixInverse(nullptr, m_MatrixView) };
	m_PickingRayWorldSpaceOrigin = XMVector3TransformCoord(ViewSpaceRayOrigin, MatrixViewInverse);
	m_PickingRayWorldSpaceDirection = XMVector3TransformNormal(ViewSpaceRayDirection, MatrixViewInverse);

	UpdatePickingRay();

	PickBoundingSphere();

	PickTriangle();
}

const char* CGameWindow::GetPickedGameObjectName()
{
	if (m_PtrPickedGameObject)
	{
		return m_PtrPickedGameObject->m_Name.c_str();
	}
	return nullptr;
}

void CGameWindow::BeginRendering(const FLOAT* ClearColor)
{
	m_DeviceContext->ClearRenderTargetView(m_RenderTargetView.Get(), ClearColor);//делаем заливку всей области заданным цветом
	m_DeviceContext->ClearDepthStencilView(m_DepthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);//очистка трафарета глубины

	ID3D11SamplerState* SamplerState{ m_CommonStates->LinearWrap() };//состояние сэмплера
	m_DeviceContext->PSSetSamplers(0, 1, &SamplerState);//установка состояния сэмплера наэтапе пиксельного шейдера

	m_DeviceContext->OMSetBlendState(m_CommonStates->AlphaBlend(), nullptr, 0xFFFFFFFF);//установка состояния смешивателя (blender-а) наэтапе слияния вывода (смешивание - обьединение пикселей с учетом прозрачности)

	//установка состояния растеризатора
	SetGameWindowCullMode();
	
	m_MatrixView = XMMatrixLookAtLH(m_PtrCurrentCamera->EyePosition, m_PtrCurrentCamera->FocusPosition, m_PtrCurrentCamera->UpDirection);//создание матрицы вида на основе текущего состояния камеры
}

void CGameWindow::AnimateGameObjects()
{
	for (auto& go : m_vGameObjects)
	{
		if (go->ComponentRender.PtrObject3D) go->ComponentRender.PtrObject3D->Animate();
	}
}

void CGameWindow::DrawGameObjects(float DeltaTime)
{
	m_DeviceContext->RSSetViewports(1, &m_vViewports[0]);//устанавливаем основной вьюпорт

	//если установлен флаг использования света, говорим об этом пиксельному шейдеру
	if (EFLAG_HAS(m_eFlagsGamerendering, EFlagsGameRendering::UseLighting))
	{
		cbPSBaseFlagsData.bUseLighting = TRUE;
	}
	//обновление позиции камеры
	cbPSBaseEyeData.EyePosition = m_PtrCurrentCamera->EyePosition;

	// Update directional light source position //TODO day/week light intensity
	// static float DirectionalLightRoll{};
	// DirectionalLightRoll += XM_2PI * DeltaTime * KSkyTimeFactorAbsolute / 4.0f;
	// if (DirectionalLightRoll >= XM_PIDIV2) DirectionalLightRoll = -XM_PIDIV2;
	// XMVECTOR DirectionalLightSourcePosition{ XMVector3TransformCoord(XMVectorSet(0, 1, 0, 1), XMMatrixRotationRollPitchYaw(0, 0, DirectionalLightRoll)) };
	// SetDirectionalLight(DirectionalLightSourcePosition);

	//обработка обьектов с прозрачной текстурой
	for (auto& go : m_vGameObjects)
	{
		if (go->ComponentRender.IsTransparent) continue;

		UpdateGameObject(go.get(), DeltaTime);
		DrawGameObjectOutlineGlowing(go.get());
		DrawGameObject(go.get());
		DrawGameObjectBoundingSphere(go.get());
	}
	//обработка обьектов с непрозрачной текстурой
	for (auto& go : m_vGameObjects)
	{
		if (!go->ComponentRender.IsTransparent) continue;

		UpdateGameObject(go.get(), DeltaTime);
		DrawGameObjectOutlineGlowing(go.get());
		DrawGameObject(go.get());
		DrawGameObjectBoundingSphere(go.get());
	}

	if (EFLAG_HAS(m_eFlagsGamerendering, EFlagsGameRendering::DrawNormals))//если установлен флаг отрисовки нормалей
	{
		m_VSBase->Use();
		m_GSNormal->Use();//применяем геометрический шейдер нормалей
		m_PSNormal->Use();//применяем пиксельный шейдер нормалей
		//рисуем нормали
		for (auto& go : m_vGameObjects)
		{
			DrawGameObjectNormal(go.get());
		}

		m_DeviceContext->GSSetShader(nullptr, nullptr, 0);//сброс геометрического шейдера
	}

	if (EFLAG_HAS(m_eFlagsGamerendering, EFlagsGameRendering::DrawMiniAxes))
	{
		DrawMiniAxes();
	}

	if (EFLAG_HAS(m_eFlagsGamerendering, EFlagsGameRendering::DrawPickingData))
	{
		DrawPickingRay();

		DrawPickedTriangle();
	}
}

void CGameWindow::DrawMiniAxes()
{
	m_DeviceContext->RSSetViewports(1, &m_vViewports[1]);//установка альтернативного вьюпорта

	m_VSBase->Use();//применяем базовый вертексный шейдер
	m_PSBase->Use();//применяем базовый пиксельный шейдер
	m_DeviceContext->GSSetShader(nullptr, nullptr, 0);//сбрасываем геометрический шейдер
	//сбрасываем константный буфер пиксельного шейдера (не используем текстуры и свет)
	cbPSBaseFlagsData.bUseTexture = FALSE;
	cbPSBaseFlagsData.bUseLighting = FALSE;
	m_PSBase->UpdateConstantBuffer(0);

	for (auto& i : m_vMiniAxisGameObjects)//перебор всех осей представления
	{
		//обновляем мировую матрицу для отрисовки осей
		cbVSSpaceData.World = XMMatrixTranspose(i->ComponentTransform.MatrixWorld);
		cbVSSpaceData.WVP = XMMatrixTranspose(i->ComponentTransform.MatrixWorld * m_MatrixView * m_MatrixProjection);
		m_VSBase->UpdateConstantBuffer(0);

		i->ComponentRender.PtrObject3D->Draw();//отрисовка

		i->ComponentTransform.Translation = m_PtrCurrentCamera->EyePosition + m_PtrCurrentCamera->Forward;//смещение обьекта (следование за камерой)
		i->UpdateWorldMatrix();//обновление мировой матрицы оси
	}

	m_DeviceContext->RSSetViewports(1, &m_vViewports[0]);//установка основного вьюпорта
}

void CGameWindow::DrawPickingRay()
{
	m_DeviceContext->RSSetState(m_CommonStates->CullNone());

	m_VSLine->Use();
	m_DeviceContext->GSSetShader(nullptr, nullptr, 0);
	m_PSLine->Use();

	cbVSSpaceData.World = XMMatrixTranspose(KMatrixIdentity);
	cbVSSpaceData.WVP = XMMatrixTranspose(KMatrixIdentity * m_MatrixView * m_MatrixProjection);
	m_VSLine->UpdateConstantBuffer(0);

	m_ObjectLinePickingRay->Draw();
}

void CGameWindow::DrawPickedTriangle()
{
	m_VSBase->Use();
	m_DeviceContext->GSSetShader(nullptr, nullptr, 0);
	m_PSNormal->Use();

	cbVSSpaceData.World = XMMatrixTranspose(KMatrixIdentity);
	cbVSSpaceData.WVP = XMMatrixTranspose(KMatrixIdentity * m_MatrixView * m_MatrixProjection);
	m_VSBase->UpdateConstantBuffer(0);

	m_Object3DPickedTriangle->m_Model.vMeshes[0].vVertices[0].Position = m_PickedTriangleV0;
	m_Object3DPickedTriangle->m_Model.vMeshes[0].vVertices[1].Position = m_PickedTriangleV1;
	m_Object3DPickedTriangle->m_Model.vMeshes[0].vVertices[2].Position = m_PickedTriangleV2;
	m_Object3DPickedTriangle->UpdateMeshBuffer();

	m_Object3DPickedTriangle->Draw();
}

void CGameWindow::SetGameState(EGameState newState) {
	m_gameState = newState;
}

EGameState CGameWindow::GetGameState() {
	return m_gameState;
}

void CGameWindow::UpdateGameObject(CGameObject* PtrGO, float DeltaTime)
{
	//logic block begin // TODO

	if (GetGameState() == EGameState::Playing && std::string(PtrGO->m_Name).find("Ghost", 0) != std::string::npos) {
		XMVECTOR Move = m_PtrCurrentCamera->EyePosition - PtrGO->ComponentTransform.Translation;
		XMFLOAT4 Move4;
		XMFLOAT4 MoveN4XZ;
		XMStoreFloat4(&Move4, Move);
		Move4.y = 0;
		Move4.w = 0;
		XMVECTOR MoveNXZ{ XMVector4Normalize(XMLoadFloat4(&Move4)) };
		XMStoreFloat4(&MoveN4XZ, MoveNXZ);
		MoveNXZ *= DeltaTime;
		XMVECTOR length = XMVector4Length(Move);
		float distance = 0.0f;
		XMStoreFloat(&distance, length);
		if (GetGameState() == EGameState::Playing && distance < 3.5f) {
			SetGameState(EGameState::GameOver);
		}
		else {
			double c1 = acos(MoveN4XZ.z);
			double c2 = -c1;
			double s1 = asin(MoveN4XZ.x);
			double s2 = MoveN4XZ.x > 0 ? XM_PI - s1 : -XM_PI - s1;
			double angle;
			double eps = 0.001;
			if (abs(c1 - s1) < eps) angle = c1;
			if (abs(c1 - s2) < eps) angle = c1;
			if (abs(c2 - s1) < eps) angle = c2;
			if (abs(c2 - s2) < eps) angle = c2;
			PtrGO->ComponentTransform.RotationQuaternion = XMQuaternionRotationRollPitchYaw(0, angle + XM_PI, 0);
			PtrGO->ComponentTransform.Translation += MoveNXZ * 3;
			PtrGO->UpdateWorldMatrix();
		}
	}

	//logic block end




	CShader* VS{ m_VSBase.get() };
	CShader* PS{ m_PSBase.get() };

	cbVSSpaceData.World = XMMatrixTranspose(PtrGO->ComponentTransform.MatrixWorld);
	cbVSSpaceData.WVP = XMMatrixTranspose(PtrGO->ComponentTransform.MatrixWorld * m_MatrixView * m_MatrixProjection);

	// Set VS
	if (PtrGO->ComponentRender.PtrObject3D->m_Model.bIsAnimated)
	{
		VS = m_VSAnimation.get();
	}
	else
	{
		// if (PtrGO == m_PtrSky || PtrGO == m_PtrCloud || PtrGO == m_PtrSun || PtrGO == m_PtrMoon)
		// {
		// 	VS = m_VSSky.get();
		// }
	}

	// Set PS
	// if (PtrGO == m_PtrSky)
	// {
	// 	PS = m_PSSky.get();
	// }
	// 
	// if (PtrGO == m_PtrSky)
	// {
	// 	static float SkyTime{ 1.0f };
	// 	static float SkyTimeFactor{ KSkyTimeFactorAbsolute };
	// 
	// 	SkyTime -= DeltaTime * SkyTimeFactor;
	// 	if (SkyTime < -1.0f) SkyTimeFactor = -SkyTimeFactor;
	// 	if (SkyTime > +1.0f) SkyTimeFactor = -SkyTimeFactor;
	// 
	// 	PtrGO->ComponentTransform.Scaling = XMVectorSet(KSkyDistance, KSkyDistance, KSkyDistance, 0);
	// 	PtrGO->ComponentTransform.Translation = m_PtrCurrentCamera->EyePosition;
	// 	PtrGO->UpdateWorldMatrix();
	// 
	// 	cbPSSkyTimeData.SkyTime = SkyTime;
	// }
	// 
	// if (PtrGO == m_PtrCloud)
	// {
	// 	static float Yaw{};
	// 	Yaw -= XM_2PI * DeltaTime * 0.01f;
	// 	if (Yaw <= -XM_2PI) Yaw = 0;
	// 
	// 	XMVECTOR Offset{ XMVector3TransformCoord(XMVectorSet(0, 0, KSkyDistance, 1), XMMatrixRotationRollPitchYaw(-XM_PIDIV4, Yaw, 0)) };
	// 	PtrGO->ComponentTransform.Translation = m_PtrCurrentCamera->EyePosition + Offset;
	// 
	// 	PtrGO->ComponentTransform.RotationQuaternion = XMQuaternionRotationRollPitchYaw(-XM_PIDIV2 - XM_PIDIV4, Yaw, 0);
	// 
	// 	PtrGO->UpdateWorldMatrix();
	// }
	// 
	// if (PtrGO == m_PtrSun)
	// {
	// 	static float SunRoll{};
	// 	SunRoll += XM_2PI * DeltaTime * KSkyTimeFactorAbsolute / 4.0f;
	// 	if (SunRoll >= XM_2PI) SunRoll = 0;
	// 
	// 	XMVECTOR Offset{ XMVector3TransformCoord(XMVectorSet(KSkyDistance, 0, 0, 1), XMMatrixRotationRollPitchYaw(0, 0, XM_PIDIV2 + SunRoll)) };
	// 	PtrGO->ComponentTransform.Translation = m_PtrCurrentCamera->EyePosition + Offset;
	// 
	// 	PtrGO->ComponentTransform.RotationQuaternion = XMQuaternionRotationRollPitchYaw(0, 0, XM_PIDIV2 + SunRoll);
	// 
	// 	PtrGO->UpdateWorldMatrix();
	// }
	// 
	// if (PtrGO == m_PtrMoon)
	// {
	// 	static float MoonRoll{};
	// 	MoonRoll += XM_2PI * DeltaTime * KSkyTimeFactorAbsolute / 4.0f;
	// 	if (MoonRoll >= XM_2PI) MoonRoll = 0;
	// 
	// 	XMVECTOR Offset{ XMVector3TransformCoord(XMVectorSet(KSkyDistance, 0, 0, 1), XMMatrixRotationRollPitchYaw(0, 0, -XM_PIDIV2 + MoonRoll)) };
	// 	PtrGO->ComponentTransform.Translation = m_PtrCurrentCamera->EyePosition + Offset;
	// 
	// 	PtrGO->ComponentTransform.RotationQuaternion = XMQuaternionRotationRollPitchYaw(0, 0, -XM_PIDIV2 + MoonRoll);
	// 
	// 	PtrGO->UpdateWorldMatrix();
	// }

	if (EFLAG_HAS(PtrGO->eFlagsGameObjectRendering, EFlagsGameObjectRendering::NoLighting))
	{
		cbPSBaseFlagsData.bUseLighting = FALSE;
	}
	else
	{
		SetGameWindowUseLighiting();
	}

	if (PtrGO->ComponentRender.PtrTexture)
	{
		cbPSBaseFlagsData.bUseTexture = TRUE;
		PtrGO->ComponentRender.PtrTexture->Use();
	}
	else
	{
		cbPSBaseFlagsData.bUseTexture = FALSE;
	}

	VS->UpdateAllConstantBuffers();
	VS->Use();
	VS->UpdateAllConstantBuffers();

	PS->UpdateAllConstantBuffers();
	PS->Use();
	PS->UpdateAllConstantBuffers();
}

void CGameWindow::UpdatePickingRay()
{
	m_ObjectLinePickingRay->vVertices[0].Position = m_PickingRayWorldSpaceOrigin;
	m_ObjectLinePickingRay->vVertices[1].Position = m_PickingRayWorldSpaceOrigin + m_PickingRayWorldSpaceDirection * KPickingRayLength;
	m_ObjectLinePickingRay->Update();
}

void CGameWindow::DrawGameObject(CGameObject* PtrGO)
{
	assert(PtrGO->ComponentRender.PtrObject3D);
	
	if (EFLAG_HAS(PtrGO->eFlagsGameObjectRendering, EFlagsGameObjectRendering::NoCulling))
	{
		m_DeviceContext->RSSetState(m_CommonStates->CullNone());//отключение отрисовки
	}
	else
	{
		SetGameWindowCullMode();//отрисовка граней в соответствии с выбранными флагами растеризатора
	}

	if (EFLAG_HAS(PtrGO->eFlagsGameObjectRendering, EFlagsGameObjectRendering::NoDepthComparison))
	{
		m_DeviceContext->OMSetDepthStencilState(m_CommonStates->DepthNone(), 0);//отключение буфера глубины
	}
	else
	{
		m_DeviceContext->OMSetDepthStencilState(m_CommonStates->DepthDefault(), 0);//обнуление значений трафарета глубины на этапе слияния-вывода OM
	}

	PtrGO->ComponentRender.PtrObject3D->Draw();
}

void CGameWindow::DrawGameObjectOutlineGlowing(CGameObject* PtrGO) 
{
	if (PtrGO->ComponentRender.IsOutlineGlowing) {

		HRESULT result;
		

		//меняем renderTarget
		m_DeviceContext->OMSetRenderTargets(1, &m_OtherRenderTargetView, m_DepthStencilView.Get());

		//сохраняем состояние буфера глубины
		//ID3D11DepthStencilState* depthStensilState;
		//m_DeviceContext->OMGetDepthStencilState(&depthStensilState, 0);
		
		
		//очищаем буфер
		m_DeviceContext->ClearRenderTargetView(m_OtherRenderTargetView, Colors::Black);//делаем заливку всей области заданным цветом
		//m_DeviceContext->ClearDepthStencilView(m_DepthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);//очистка трафарета глубины


		//рендерим трафарет модели
		m_DeviceContext->RSSetState(m_CommonStates->CullNone());
		m_VSBase->Use();
		m_PSStencil->Use();
		PtrGO->ComponentRender.PtrObject3D->Draw();
		//for (size_t iMesh = 0; iMesh < PtrGO->ComponentRender.PtrObject3D->m_Model.vMeshes.size(); ++iMesh)
		//{
		//	const SMesh& Mesh{ PtrGO->ComponentRender.PtrObject3D->m_Model.vMeshes[iMesh] };
		//	const SMaterial& Material{ PtrGO->ComponentRender.PtrObject3D->m_Model.vMaterials[Mesh.MaterialID] };
		//
		//	m_DeviceContext->IASetIndexBuffer(PtrGO->ComponentRender.PtrObject3D->m_vMeshBuffers[iMesh].IndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);//привязка индексного буфера к этапу IA
		//	m_DeviceContext->IASetVertexBuffers(0, 1, PtrGO->ComponentRender.PtrObject3D->m_vMeshBuffers[iMesh].VertexBuffer.GetAddressOf(),//привязка вертексного буфера к этапу IA
		//		&PtrGO->ComponentRender.PtrObject3D->m_vMeshBuffers[iMesh].VertexBufferStride, &PtrGO->ComponentRender.PtrObject3D->m_vMeshBuffers[iMesh].VertexBufferOffset);
		//
		//	if (PtrGO->ComponentRender.PtrObject3D->m_Model.bIsAnimated)
		//	{
		//		m_DeviceContext->IASetVertexBuffers(1, 1, PtrGO->ComponentRender.PtrObject3D->m_vMeshBuffers[iMesh].VertexBufferAnimation.GetAddressOf(),
		//			&PtrGO->ComponentRender.PtrObject3D->m_vMeshBuffers[iMesh].VertexBufferAnimationStride, &PtrGO->ComponentRender.PtrObject3D->m_vMeshBuffers[iMesh].VertexBufferAnimationOffset);
		//	}
		//
		//	m_DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);//указываем информацию о типе примитива и порядке данных которые описывают входные данные этапа IA (что это полигоны)
		//
		//	m_DeviceContext->DrawIndexed(static_cast<UINT>(Mesh.vTriangles.size() * 3), 0, 0);//Рисуем индексированные примитивы
		//}
		
		//сохраняем состояние заднего буфера в pTextureOther1
		ID3D11Texture2D* BackBuffer2{};
		m_SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&BackBuffer2);//получаем указатель на 1-й задний буфер подкачки
		m_DeviceContext->CopyResource(pTextureOther1, pTextureRenderTarget);
		BackBuffer2->Release();

		//тут где-то утечка памяти
		////получаем текстуру модели
		//D3D11_TEXTURE2D_DESC texDesc;
		//ID3D11Texture2D* textureStencil;
		//ZeroMemory(&texDesc, sizeof(D3D11_TEXTURE2D_DESC));
		//texDesc.Width = 1000;//TODO hardcode
		//texDesc.Height = 600;
		//texDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		//texDesc.Usage = D3D11_USAGE_DEFAULT;
		//texDesc.SampleDesc.Count = 1;
		//texDesc.SampleDesc.Quality = 0;
		//texDesc.CPUAccessFlags = 0;
		//texDesc.ArraySize = 1;
		//texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
		//texDesc.MiscFlags = 0;
		//texDesc.MipLevels = 1;
		//
		//m_Device->CreateTexture2D(&texDesc, NULL, &textureStencil);
		
		
		//вовзращаем состояние заднего буфера
		m_DeviceContext->OMSetRenderTargets(1, m_RenderTargetView.GetAddressOf(), m_DepthStencilView.Get());
		m_PSBase->Use();


		//рендер свечения обьекта
		D3D11_TEXTURE2D_DESC textDesc{};
		pTextureOther1->GetDesc(&textDesc);
		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
		srvDesc.Format = textDesc.Format;//DXGI_FORMAT_R8G8B8A8_UNORM
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MostDetailedMip = 0;
		srvDesc.Texture2D.MipLevels = 1;
		ID3D11ShaderResourceView* shaderView = nullptr;
		m_Device->CreateShaderResourceView(pTextureOther1, &srvDesc, &shaderView);
		m_DeviceContext->PSSetShaderResources(0, 1, &shaderView);
		m_PSOutlineGlowing->Use();
		XMVECTOR Move = m_PtrCurrentCamera->EyePosition - PtrGO->ComponentTransform.Translation;
		XMFLOAT4 Move4;
		XMFLOAT4 MoveN4XZ;
		XMStoreFloat4(&Move4, Move);
		Move4.y = 0;
		Move4.w = 0;
		XMVECTOR MoveNXZ{ XMVector4Normalize(XMLoadFloat4(&Move4)) };
		XMStoreFloat4(&MoveN4XZ, MoveNXZ);
		XMVECTOR length = XMVector4Length(Move);
		float distance = 0.0f;
		XMStoreFloat(&distance, length);
		double c1 = acos(MoveN4XZ.z);
		double c2 = -c1;
		double s1 = asin(MoveN4XZ.x);
		double s2 = MoveN4XZ.x > 0 ? XM_PI - s1 : -XM_PI - s1;
		double angle;
		double eps = 0.001;
		if (abs(c1 - s1) < eps) angle = c1;
		if (abs(c1 - s2) < eps) angle = c1;
		if (abs(c2 - s1) < eps) angle = c2;
		if (abs(c2 - s2) < eps) angle = c2;
		m_2dFrame->ComponentTransform.Translation = m_PtrCurrentCamera->EyePosition;
		m_2dFrame->ComponentTransform.RotationQuaternion = XMQuaternionRotationRollPitchYaw(0, angle + XM_PI, 0);
		m_2dFrame->UpdateWorldMatrix();
		cbVSSpaceData.World = XMMatrixTranspose(m_2dFrame->ComponentTransform.MatrixWorld);
		cbVSSpaceData.WVP = XMMatrixTranspose(m_2dFrame->ComponentTransform.MatrixWorld * m_MatrixView * m_MatrixProjection);
		m_VSBase->UpdateAllConstantBuffers();
		m_VSBase->Use();
		DrawGameObject(m_2dFrame);

		
		////возвращаем состояние буфера глубины
		//m_DeviceContext->OMSetDepthStencilState(depthStensilState, 0);


		cbVSSpaceData.World = XMMatrixTranspose(PtrGO->ComponentTransform.MatrixWorld);
		cbVSSpaceData.WVP = XMMatrixTranspose(PtrGO->ComponentTransform.MatrixWorld * m_MatrixView * m_MatrixProjection);
		m_VSBase->UpdateAllConstantBuffers();
	}
}

void CGameWindow::DrawGameObjectNormal(CGameObject* PtrGO)
{
	if (PtrGO->ComponentRender.PtrObject3D)
	{
		if (PtrGO->ComponentRender.PtrObject3D->m_Model.bIsAnimated)
		{
			m_VSAnimation->Use();
		}
		else
		{
			m_VSBase->Use();
		}

		PtrGO->ComponentRender.PtrObject3D->DrawNormals();
	}
}

void CGameWindow::DrawGameObjectBoundingSphere(CGameObject* PtrGO)
{
	if (EFLAG_HAS(m_eFlagsGamerendering, EFlagsGameRendering::DrawBoundingSphere))
	{
		// if (PtrGO == m_PtrSky || PtrGO == m_PtrCloud || PtrGO == m_PtrSun || PtrGO == m_PtrMoon) return; // TODO sky

		m_VSBase->Use();

		XMMATRIX Translation{ XMMatrixTranslationFromVector(PtrGO->ComponentTransform.Translation + PtrGO->ComponentPhysics.BoundingSphere.CenterOffset) };
		XMMATRIX Scaling{ XMMatrixScaling(PtrGO->ComponentPhysics.BoundingSphere.Radius,
			PtrGO->ComponentPhysics.BoundingSphere.Radius, PtrGO->ComponentPhysics.BoundingSphere.Radius) };
		XMMATRIX World{ Scaling * Translation };
		cbVSSpaceData.World = XMMatrixTranspose(World);
		cbVSSpaceData.WVP = XMMatrixTranspose(World * m_MatrixView * m_MatrixProjection);
		m_VSBase->UpdateConstantBuffer(0);

		m_DeviceContext->RSSetState(m_CommonStates->Wireframe());

		m_Object3DBoundingSphere->Draw();

		SetGameWindowCullMode();
	}
}

void CGameWindow::SetGameWindowCullMode()
{
	switch (m_eRasterizerState)
	{
	case ERasterizerState::CullNone:
		m_DeviceContext->RSSetState(m_CommonStates->CullNone());
		break;
	case ERasterizerState::CullClockwise:
		m_DeviceContext->RSSetState(m_CommonStates->CullClockwise());
		break;
	case ERasterizerState::CullCounterClockwise:
		m_DeviceContext->RSSetState(m_CommonStates->CullCounterClockwise());
		break;
	case ERasterizerState::WireFrame:
		m_DeviceContext->RSSetState(m_CommonStates->Wireframe());
		break;
	default:
		break;
	}
}

void CGameWindow::SetGameWindowUseLighiting()
{
	if (EFLAG_HAS(m_eFlagsGamerendering, EFlagsGameRendering::UseLighting))
	{
		cbPSBaseFlagsData.bUseLighting = TRUE;
	}
}

void CGameWindow::EndRendering()
{
	m_SwapChain->Present(0, 0);//делает свап переднего буфера и заднего
}

Keyboard::State CGameWindow::GetKeyState()
{
	return m_Keyboard->GetState();
}

Mouse::State CGameWindow::GetMouseState()
{
	Mouse::State ResultState{ m_Mouse->GetState() };

	m_Mouse->ResetScrollWheelValue();//сброс значения колесика прокрутки

	return ResultState;
}