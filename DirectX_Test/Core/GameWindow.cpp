#include "GameWindow.h"

//структура доступа для вертексного шейдера (элемент ввода для этапа IA)
constexpr D3D11_INPUT_ELEMENT_DESC KBaseInputElementDescs[]
{
	{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT	, 0,  0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "COLOR"	, 0, DXGI_FORMAT_R32G32B32A32_FLOAT	, 0, 16, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "TEXCOORD", 0, DXGI_FORMAT_R32G32B32A32_FLOAT	, 0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "NORMAL"	, 0, DXGI_FORMAT_R32G32B32A32_FLOAT	, 0, 48, D3D11_INPUT_PER_VERTEX_DATA, 0 },
};

void CGameWindow::CreateWin32(WNDPROC WndProc, LPCTSTR WindowName, const wstring& FontFileName, bool bWindowed)
{
	CreateWin32Window(WndProc, WindowName);

	InitializeDirectX(FontFileName, bWindowed);
}

void CGameWindow::SetPerspective(float FOV, float NearZ, float FarZ)
{
	m_MatrixProjection = XMMatrixPerspectiveFovLH(FOV, m_WindowSize.x / m_WindowSize.y, NearZ, FarZ);
}

void CGameWindow::SetGameRenderingFlags(EFlagsGameRendering Flags)
{
	m_eFlagsGamerendering = Flags;
}

void CGameWindow::ToggleGameRenderingFlags(EFlagsGameRendering Flags)
{
	m_eFlagsGamerendering ^= Flags;
}

void CGameWindow::SetDirectionalLight(const XMVECTOR& LightSourcePosition, const XMVECTOR& Color)
{
	m_cbPSBaseLightsData.DirectionalLight = XMVector3Normalize(LightSourcePosition);
	m_cbPSBaseLightsData.DirectionalColor = Color;

	UpdateCBPSBaseLights();
}

void CGameWindow::SetAmbientlLight(const XMFLOAT3& Color, float Intensity)
{
	m_cbPSBaseLightsData.AmbientColor = Color;
	m_cbPSBaseLightsData.AmbientIntensity = Intensity;

	UpdateCBPSBaseLights();
}

void CGameWindow::AddCamera(const SCameraData& CameraData)
{
	m_vCameras.emplace_back(CameraData);
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

	CreateBaseShaders();

	CreateCBs();

	CreateMiniAxes();

	m_SpriteBatch = make_unique<SpriteBatch>(m_DeviceContext.Get());
	m_SpriteFont = make_unique<SpriteFont>(m_Device.Get(), FontFileName.c_str());
	m_CommonStates = make_unique<CommonStates>(m_Device.Get());
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
	D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0, nullptr, 0, D3D11_SDK_VERSION,
		&SwapChainDesc, &m_SwapChain, &m_Device, nullptr, &m_DeviceContext);
}

void CGameWindow::CreateSetViews()
{
	ComPtr<ID3D11Texture2D> BackBuffer{};
	m_SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), &BackBuffer);//получаем указатель на 1-йзадний буфер подкачки

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

	m_Mouse->SetMode(Mouse::Mode::MODE_RELATIVE);
}

void CGameWindow::CreateBaseShaders()
{
	m_VSBase = make_unique<CShader>(m_Device.Get(), m_DeviceContext.Get());
	m_VSBase->Create(EShaderType::VertexShader, L"Shader\\VSBase.hlsl", "main", KBaseInputElementDescs, ARRAYSIZE(KBaseInputElementDescs));

	m_PSBase = make_unique<CShader>(m_Device.Get(), m_DeviceContext.Get());
	m_PSBase->Create(EShaderType::PixelShader, L"Shader\\PSBase.hlsl", "main");

	m_GSNormal = make_unique<CShader>(m_Device.Get(), m_DeviceContext.Get());
	m_GSNormal->Create(EShaderType::GeometryShader, L"Shader\\GSNormal.hlsl", "main");

	m_PSNormal = make_unique<CShader>(m_Device.Get(), m_DeviceContext.Get());
	m_PSNormal->Create(EShaderType::PixelShader, L"Shader\\PSNormal.hlsl", "main");
}

void CGameWindow::CreateCBs()
{
	CreateCB(sizeof(SCBVSBaseSpaceData), m_cbVSBaseSpace.GetAddressOf());

	CreateCB(sizeof(SCBPSBaseFlagsData), m_cbPSBaseFlags.GetAddressOf());
	CreateCB(sizeof(SCBPSBaseLightsData), m_cbPSBaseLights.GetAddressOf());
	CreateCB(sizeof(SComponentRender::SMaterial), m_cbPSBaseMaterial.GetAddressOf());
	CreateCB(sizeof(SCBPSBaseEyeData), m_cbPSBaseEye.GetAddressOf());

	UpdateCBPSBaseFlags();
	UpdateCBPSBaseLights();
	UpdateCBPSBaseEye();
}

void CGameWindow::CreateMiniAxes()
{
	m_vMiniAxisObject3Ds.emplace_back(make_unique<CObject3D>(m_Device.Get(), m_DeviceContext.Get()));
	m_vMiniAxisObject3Ds.emplace_back(make_unique<CObject3D>(m_Device.Get(), m_DeviceContext.Get()));
	m_vMiniAxisObject3Ds.emplace_back(make_unique<CObject3D>(m_Device.Get(), m_DeviceContext.Get()));

	m_vMiniAxisObject3Ds[0]->Create(GenerateCone(0, 16, XMVectorSet(1, 0, 0, 1)));
	m_vMiniAxisObject3Ds[1]->Create(GenerateCone(0, 16, XMVectorSet(0, 1, 0, 1)));
	m_vMiniAxisObject3Ds[2]->Create(GenerateCone(0, 16, XMVectorSet(0, 0, 1, 1)));

	m_vMiniAxisGameObjects.emplace_back(make_unique<CGameObject>());
	m_vMiniAxisGameObjects.emplace_back(make_unique<CGameObject>());
	m_vMiniAxisGameObjects.emplace_back(make_unique<CGameObject>());
	m_vMiniAxisGameObjects[0]->ComponentRender.Material.MaterialDiffuse = XMFLOAT3(1, 0, 0);
	m_vMiniAxisGameObjects[0]->ComponentRender.PtrObject3D = m_vMiniAxisObject3Ds[0].get();
	m_vMiniAxisGameObjects[0]->ComponentTransform.Rotation = XMQuaternionRotationRollPitchYaw(0, 0, -XM_PIDIV2);
	m_vMiniAxisGameObjects[1]->ComponentRender.Material.MaterialDiffuse = XMFLOAT3(0, 1, 0);
	m_vMiniAxisGameObjects[1]->ComponentRender.PtrObject3D = m_vMiniAxisObject3Ds[1].get();
	m_vMiniAxisGameObjects[2]->ComponentRender.Material.MaterialDiffuse = XMFLOAT3(0, 0, 1);
	m_vMiniAxisGameObjects[2]->ComponentRender.PtrObject3D = m_vMiniAxisObject3Ds[2].get();
	m_vMiniAxisGameObjects[2]->ComponentTransform.Rotation = XMQuaternionRotationRollPitchYaw(0, -XM_PIDIV2, -XM_PIDIV2);

	m_vMiniAxisGameObjects[0]->ComponentTransform.Scaling =
		m_vMiniAxisGameObjects[1]->ComponentTransform.Scaling =
		m_vMiniAxisGameObjects[2]->ComponentTransform.Scaling = XMVectorSet(0.1f, 1.0f, 0.1f, 0);

	m_vMiniAxisGameObjects[0]->UpdateWorldMatrix();
	m_vMiniAxisGameObjects[1]->UpdateWorldMatrix();
	m_vMiniAxisGameObjects[2]->UpdateWorldMatrix();
}

void CGameWindow::CreateCB(size_t ByteWidth, ID3D11Buffer** ppBuffer)
{
	D3D11_BUFFER_DESC BufferDesc{};
	BufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;//тип буфера (константный)
	BufferDesc.ByteWidth = static_cast<UINT>(ByteWidth);//размер буфера
	BufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;//доступ к буферу со стороны ЦПУ
	BufferDesc.MiscFlags = 0;//прочие флаги
	BufferDesc.StructureByteStride = 0;//размер структуры (0 для неструктурированных буферов)
	BufferDesc.Usage = D3D11_USAGE_DYNAMIC;//режим доступа (сейчас ЦПУ - запись, ГПУ - чтение)

	m_Device->CreateBuffer(&BufferDesc, nullptr, ppBuffer);
}

void CGameWindow::UpdateCBVSBaseSpace(const XMMATRIX& MatrixWorld)
{
	m_cbVSBaseSpaceData.WVP = XMMatrixTranspose(MatrixWorld * m_MatrixView * m_MatrixProjection);//считаем матрицу WVP
	m_cbVSBaseSpaceData.World = XMMatrixTranspose(MatrixWorld);

	UpdateCB(sizeof(SCBVSBaseSpaceData), m_cbVSBaseSpace.Get(), &m_cbVSBaseSpaceData);//обновляем буфер для гпу

	m_DeviceContext->VSSetConstantBuffers(0, 1, m_cbVSBaseSpace.GetAddressOf());//устанавливает константный буфер для вертексного шейдера
}

void CGameWindow::UpdateCBPSBaseFlags()
{
	UpdateCB(sizeof(SCBPSBaseFlagsData), m_cbPSBaseFlags.Get(), &m_cbPSBaseFlagsData);

	m_DeviceContext->PSSetConstantBuffers(0, 1, m_cbPSBaseFlags.GetAddressOf());//устанавливает константный буфер для пиксельного шейдера
}

void CGameWindow::UpdateCBPSBaseLights()
{
	UpdateCB(sizeof(SCBPSBaseLightsData), m_cbPSBaseLights.Get(), &m_cbPSBaseLightsData);

	m_DeviceContext->PSSetConstantBuffers(1, 1, m_cbPSBaseLights.GetAddressOf());
}

void CGameWindow::UpdateCBPSBaseMaterial(const SComponentRender::SMaterial& PtrMaterial)
{
	UpdateCB(sizeof(SComponentRender::SMaterial), m_cbPSBaseMaterial.Get(), &PtrMaterial);

	m_DeviceContext->PSSetConstantBuffers(2, 1, m_cbPSBaseMaterial.GetAddressOf());
}

void CGameWindow::UpdateCBPSBaseEye()
{
	UpdateCB(sizeof(SCBPSBaseEyeData), m_cbPSBaseEye.Get(), &m_cbPSBaseEyeData);

	m_DeviceContext->PSSetConstantBuffers(3, 1, m_cbPSBaseEye.GetAddressOf());
}

void CGameWindow::UpdateCB(size_t ByteWidth, ID3D11Buffer* pBuffer, const void* pValue)
{
	D3D11_MAPPED_SUBRESOURCE MappedSubresource{};
	if (SUCCEEDED(m_DeviceContext->Map(pBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedSubresource)))//запрещаем доступ графическому процессору к константному буферу
	{
		memcpy(MappedSubresource.pData, pValue, ByteWidth);//копируем новое значение константного буфера

		m_DeviceContext->Unmap(pBuffer, 0);//вновь разрешаем доступ графическому процессору к константному буферу
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
	m_vObject3Ds.emplace_back(make_unique<CObject3D>(m_Device.Get(), m_DeviceContext.Get()));
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

CGameObject* CGameWindow::AddGameObject()
{
	m_vGameObjects.emplace_back(make_unique<CGameObject>());
	return m_vGameObjects.back().get();
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

void CGameWindow::BeginRendering(const FLOAT* ClearColor)
{
	m_DeviceContext->ClearRenderTargetView(m_RenderTargetView.Get(), ClearColor);//делаем заливку всей области заданным цветом
	m_DeviceContext->ClearDepthStencilView(m_DepthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);//очистка трафарета глубины
	
	m_DeviceContext->OMSetDepthStencilState(m_CommonStates->DepthDefault(), 0);//обнуление значений трафарета глубины на этапе слияния-вывода OM

	ID3D11SamplerState* SamplerState{ m_CommonStates->LinearWrap() };//состояние сэмплера
	m_DeviceContext->PSSetSamplers(0, 1, &SamplerState);//установка состояния сэмплера наэтапе пиксельного шейдера

	m_DeviceContext->OMSetBlendState(m_CommonStates->AlphaBlend(), nullptr, 0xFFFFFFFF);//установка состояния смешивателя (blender-а) наэтапе слияния вывода (смешивание - обьединение пикселей с учетом прозрачности)

	//установка состояния растеризатора
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
	
	m_MatrixView = XMMatrixLookAtLH(m_PtrCurrentCamera->EyePosition, m_PtrCurrentCamera->FocusPosition, m_PtrCurrentCamera->UpDirection);//создание матрицы вида на основе текущего состояния камеры
}

void CGameWindow::DrawGameObjects()
{
	m_DeviceContext->RSSetViewports(1, &m_vViewports[0]);//устанавливаем основной вьюпорт

	m_VSBase->Use();//применяем базовый вертексный шейдер
	m_PSBase->Use();//применяем базовый пиксельный шейдер
	//если установлен флаг использования света, говорим об этом пиксельному шейдеру
	if ((m_eFlagsGamerendering & EFlagsGameRendering::UseLighting) == EFlagsGameRendering::UseLighting)
	{
		m_cbPSBaseFlagsData.bUseLighting = TRUE;
	}
	//обновление позиции камеры
	m_cbPSBaseEyeData.EyePosition = m_PtrCurrentCamera->EyePosition;
	UpdateCBPSBaseEye();
	//обработка обьектов с прозрачной текстурой
	for (auto& go : m_vGameObjects)
	{
		if (go->ComponentRender.IsTransparent) continue;

		DrawGameObject(go.get());
	}
	//обработка обьектов с непрозрачной текстурой
	for (auto& go : m_vGameObjects)
	{
		if (!go->ComponentRender.IsTransparent) continue;

		DrawGameObject(go.get());
	}

	if ((m_eFlagsGamerendering & EFlagsGameRendering::DrawNormals) == EFlagsGameRendering::DrawNormals)//если установлен флаг отрисовки нормалей
	{
		m_GSNormal->Use();//применяем геометрический шейдер нормалей
		m_PSNormal->Use();//применяем пиксельный шейдер нормалей
		//рисуем нормали
		for (auto& go : m_vGameObjects)
		{
			DrawGameObjectNormal(go.get());
		}

		m_DeviceContext->GSSetShader(nullptr, nullptr, 0);//сброс геометрического шейдера
	}
}

void CGameWindow::DrawMiniAxes()
{
	m_DeviceContext->RSSetViewports(1, &m_vViewports[1]);//установка альтернативного вьюпорта

	m_VSBase->Use();//применяем базовый вертексный шейдер
	m_PSBase->Use();//применяем базовый пиксельный шейдер
	m_DeviceContext->GSSetShader(nullptr, nullptr, 0);//сбрасываем геометрический шейдер

	m_cbPSBaseFlagsData.bUseTexture = FALSE;
	m_cbPSBaseFlagsData.bUseLighting = FALSE;
	UpdateCBPSBaseFlags();//сбрасываем константный буфер пиксельного шейдера (не используем текстуры и свет)

	for (auto& i : m_vMiniAxisGameObjects)//перебор всех осей представления
	{
		UpdateCBPSBaseMaterial(i->ComponentRender.Material);//обновляем материал у всех обьектов

		UpdateCBVSBaseSpace(i->ComponentTransform.MatrixWorld);//обновляем мировую матрицу у всех обьектов

		i->ComponentRender.PtrObject3D->Draw();//отрисовка

		i->ComponentTransform.Translation = m_PtrCurrentCamera->EyePosition + m_PtrCurrentCamera->Forward;//смещение обьекта (следование за камерой)
		i->UpdateWorldMatrix();//обновление мировой матрицы оси
	}

	m_DeviceContext->RSSetViewports(1, &m_vViewports[0]);//установка основного вьюпорта
}

void CGameWindow::DrawGameObject(CGameObject* PtrGO)
{
	UpdateCBVSBaseSpace(PtrGO->ComponentTransform.MatrixWorld);//обновляем константный буфер WVP вертексного шейдера

	UpdateCBPSBaseMaterial(PtrGO->ComponentRender.Material);//обновляем константный буфер материала вертексного шейдера

	if (PtrGO->ComponentRender.PtrTexture)//если есть указатель на текстуру
	{
		m_cbPSBaseFlagsData.bUseTexture = TRUE;//обновляем константный буфер пиксельного шейдера (текстуры), указываем что она есть
		PtrGO->ComponentRender.PtrTexture->Use();//привязываем шейдер и текстуру к эт
	}
	else
	{
		m_cbPSBaseFlagsData.bUseTexture = FALSE;//обновляем константный буфер пиксельного шейдера (текстуры), указываем что ее нет
	}
	UpdateCBPSBaseFlags();

	if (PtrGO->ComponentRender.PtrObject3D)
	{
		PtrGO->ComponentRender.PtrObject3D->Draw();//отрисовываем полигоны
	}
}

void CGameWindow::DrawGameObjectNormal(CGameObject* PtrGO)
{
	UpdateCBVSBaseSpace(PtrGO->ComponentTransform.MatrixWorld);

	if (PtrGO->ComponentRender.PtrObject3D)
	{
		PtrGO->ComponentRender.PtrObject3D->DrawNormals();
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