#include "Shader.h"

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
	default:
		break;
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
	default:
		break;
	}
}