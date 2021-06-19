#include "Shader.h"

void CShader::CConstantBuffer::Create(EShaderType ShaderType, const void* PtrData, size_t DataByteWidth)
{
	assert(PtrData);

	m_eShaderType = ShaderType;
	m_PtrData = PtrData;
	m_DataByteWidth = DataByteWidth;

	D3D11_BUFFER_DESC BufferDesc{};
	BufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;//тип буфера (константный)
	BufferDesc.ByteWidth = static_cast<UINT>(m_DataByteWidth);//размер буфера
	BufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;//доступ к буферу со стороны ЦПУ
	BufferDesc.MiscFlags = 0;//прочие флаги
	BufferDesc.StructureByteStride = 0;//размер структуры (0 для неструктурированных буферов)
	BufferDesc.Usage = D3D11_USAGE_DYNAMIC;//режим доступа (сейчас ЦПУ - запись, ГПУ - чтение)

	m_PtrDevice->CreateBuffer(&BufferDesc, nullptr, m_ConstantBuffer.GetAddressOf());
}

void CShader::CConstantBuffer::Update()
{
	D3D11_MAPPED_SUBRESOURCE MappedSubresource{};
	if (SUCCEEDED(m_PtrDeviceContext->Map(m_ConstantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedSubresource)))
	{
		memcpy(MappedSubresource.pData, m_PtrData, m_DataByteWidth);

		m_PtrDeviceContext->Unmap(m_ConstantBuffer.Get(), 0);
	}
}

void CShader::CConstantBuffer::Use(UINT Slot) const
{
	switch (m_eShaderType)
	{
	case EShaderType::VertexShader:
		m_PtrDeviceContext->VSSetConstantBuffers(Slot, 1, m_ConstantBuffer.GetAddressOf());
		break;
	case EShaderType::PixelShader:
		m_PtrDeviceContext->PSSetConstantBuffers(Slot, 1, m_ConstantBuffer.GetAddressOf());
		break;
	case EShaderType::GeometryShader:
		m_PtrDeviceContext->GSSetConstantBuffers(Slot, 1, m_ConstantBuffer.GetAddressOf());
		break;
	default:
		break;
	}
}

void CShader::Create(EShaderType Type, const wstring& FileName, const string& EntryPoint,
	const D3D11_INPUT_ELEMENT_DESC* InputElementDescs, UINT NumElements)
{
	//если был указан элемент этапа ввода-ассемблера, то проверяем, вертексный ли это шейдер
	if (InputElementDescs) assert(Type == EShaderType::VertexShader);

	m_ShaderType = Type;//присваиваем локальное значение типа шейдера полю класса

	switch (m_ShaderType)
	{
	case EShaderType::VertexShader://если тип шейдера - вертексный
		//проверяем существование элемента этапа ввода-ассемблера
		assert(InputElementDescs);

		//компилиируем код HLSL в в байт-код
		D3DCompileFromFile(FileName.c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, EntryPoint.c_str(),
			"vs_4_0", D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, 0, &m_Blob, nullptr);

		//создаем обьект вершинного шейдера из скомпилированного кода
		m_PtrDevice->CreateVertexShader(m_Blob->GetBufferPointer(), m_Blob->GetBufferSize(), nullptr, &m_VertexShader);

		//создаем обьект макета ввода для описания данных буфера ввода на этапе ввода-ассемблера
		m_PtrDevice->CreateInputLayout(InputElementDescs, NumElements,
			m_Blob->GetBufferPointer(), m_Blob->GetBufferSize(), &m_InputLayout);
		break;

	case EShaderType::PixelShader://если тип шейдера - пиксельный
		//компилиируем код HLSL в в байт-код
		D3DCompileFromFile(FileName.c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, EntryPoint.c_str(),
			"ps_4_0", D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, 0, &m_Blob, nullptr);
		//создаем обьект пиксельного шейдера из скомпилированного кода
		m_PtrDevice->CreatePixelShader(m_Blob->GetBufferPointer(), m_Blob->GetBufferSize(), nullptr, &m_PixelShader);
		break;
	case EShaderType::GeometryShader://если тип шейдера - геометрический
		//компилиируем код HLSL в в байт-код
		D3DCompileFromFile(FileName.c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, EntryPoint.c_str(),
			"gs_4_0", D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, 0, &m_Blob, nullptr);
		//создаем обьект геометрического шейдера из скомпилированного кода
		m_PtrDevice->CreateGeometryShader(m_Blob->GetBufferPointer(), m_Blob->GetBufferSize(), nullptr, &m_GeometryShader);
	default:
		break;
	}
}

void CShader::AddConstantBuffer(const void* PtrData, size_t DataByteWidth)
{
	m_vConstantBuffers.emplace_back(make_unique<CConstantBuffer>(m_PtrDevice, m_PtrDeviceContext));
	m_vConstantBuffers.back()->Create(m_ShaderType, PtrData, DataByteWidth);
}

void CShader::UpdateConstantBuffer(size_t ConstantBufferIndex)
{
	if (ConstantBufferIndex >= m_vConstantBuffers.size()) return;

	m_vConstantBuffers[ConstantBufferIndex]->Update();
}

void CShader::UpdateAllConstantBuffers()
{
	for (auto& CB : m_vConstantBuffers)
	{
		CB->Update();
	}
}

void CShader::Use()
{
	switch (m_ShaderType)
	{
	case EShaderType::VertexShader://если тип шейдера - вертексный
		//устанавливает вершинный шейдер для контекста устройства
		m_PtrDeviceContext->VSSetShader(m_VertexShader.Get(), nullptr, 0);
		//привязка обьекта массива ввода кэтапу IA
		m_PtrDeviceContext->IASetInputLayout(m_InputLayout.Get());
		break;
	case EShaderType::PixelShader://если тип шейдера - пиксельный
		//устанавливает пиксельный шейдер для контекста устройства
		m_PtrDeviceContext->PSSetShader(m_PixelShader.Get(), nullptr, 0);
		break;
	case EShaderType::GeometryShader://если тип шейдера - геометрический
		//устанавливает геометрический шейдер для контекста устройства
		m_PtrDeviceContext->GSSetShader(m_GeometryShader.Get(), nullptr, 0);
		break;
	default:
		break;
	}

	for (size_t iCB = 0; iCB < m_vConstantBuffers.size(); ++iCB)
	{
		m_vConstantBuffers[iCB]->Use(static_cast<UINT>(iCB));
	}
}