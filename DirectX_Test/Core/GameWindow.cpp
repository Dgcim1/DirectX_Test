#include "GameWindow.h"

void CGameWindow::CreateWin32(WNDPROC WndProc, LPCTSTR WindowName, bool bWindowed)
{
	CreateWin32Window(WndProc, WindowName);

	InitializeDirectX(bWindowed);
}

void CGameWindow::SetPerspective(float FOV, float NearZ, float FarZ)
{
	m_MatrixProjection = XMMatrixPerspectiveFovLH(FOV, m_WindowSize.x / m_WindowSize.y, NearZ, FarZ);
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

void CGameWindow::InitializeDirectX(bool bWindowed)
{
	CreateSwapChain(bWindowed);

	CreateSetViews();

	SetViewports();

	SetPerspective(KDefaultFOV, KDefaultNearZ, KDefaultFarZ);

	CreateInputDevices();

	CreateCBWVP();
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
	//задаем массив размеров области просмотра (у нас массив из 1 элемента)
	vector<D3D11_VIEWPORT> vViewPorts{};
	{
		vViewPorts.emplace_back();

		D3D11_VIEWPORT& ViewPort{ vViewPorts.back() };
		ViewPort.TopLeftX = 0.0f;//положение Х для крайней левой точки
		ViewPort.TopLeftY = 0.0f;//положение Y для крайней верхней точки
		ViewPort.Width = m_WindowSize.x;//ширина области просмотра
		ViewPort.Height = m_WindowSize.y;//высота области просмотра
		ViewPort.MinDepth = 0.0f;//минимальная глубина области просмотра
		ViewPort.MaxDepth = 1.0f;//максимальная глубина области просмотра
	}
	//устанавливаем массив окон просмотра к этапу растеризации конвейера
	m_DeviceContext->RSSetViewports(static_cast<UINT>(vViewPorts.size()), &vViewPorts[0]);
}

void CGameWindow::CreateInputDevices()
{
	m_Keyboard = make_unique<Keyboard>();

	m_Mouse = make_unique<Mouse>();
	m_Mouse->SetWindow(m_hWnd);

	m_Mouse->SetMode(Mouse::Mode::MODE_RELATIVE);
}

void CGameWindow::CreateCBWVP()
{
	D3D11_BUFFER_DESC cbWVPDesc{};
	cbWVPDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbWVPDesc.ByteWidth = sizeof(XMMATRIX);
	cbWVPDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cbWVPDesc.MiscFlags = 0;
	cbWVPDesc.StructureByteStride = 0;
	cbWVPDesc.Usage = D3D11_USAGE_DYNAMIC;

	m_Device->CreateBuffer(&cbWVPDesc, nullptr, &m_CBWVP);
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

void CGameWindow::BeginRendering(const FLOAT* ClearColor)
{
	m_DeviceContext->ClearRenderTargetView(m_RenderTargetView.Get(), ClearColor);//делаем заливку всей области заданным цветом
	m_DeviceContext->ClearDepthStencilView(m_DepthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);//очистка трафарета глубины
	m_MatrixView = XMMatrixLookAtLH(m_PtrCurrentCamera->EyePosition, m_PtrCurrentCamera->FocusPosition, m_PtrCurrentCamera->UpDirection);//создание матрицы вида на основе текущего состояния камеры
}

void CGameWindow::UpdateCBWVP(const XMMATRIX& MatrixWorld)
{
	XMMATRIX MatrixWVP{ XMMatrixTranspose(MatrixWorld * m_MatrixView * m_MatrixProjection) };//создает матрицу WVP

	D3D11_MAPPED_SUBRESOURCE MappedSubresource{};

	//запрещаем доступ графическому процессору к константному буферу
	if (SUCCEEDED(m_DeviceContext->Map(m_CBWVP.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedSubresource)))
	{
		memcpy(MappedSubresource.pData, &MatrixWVP, sizeof(XMMATRIX));//копируем новое значение константного буфера

		m_DeviceContext->Unmap(m_CBWVP.Get(), 0);//вновь разрешаем доступ графическому процессору к константному буферу
	}

	m_DeviceContext->VSSetConstantBuffers(0, 1, m_CBWVP.GetAddressOf());//устанавливает константный буфер
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