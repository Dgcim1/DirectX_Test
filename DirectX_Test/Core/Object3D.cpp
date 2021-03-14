#include "Object3D.h"

void CObject3D::Create(const SObject3DData& Object3DData)
{
	m_Object3DData = Object3DData;//присваиваем локальное значение полю класса
	
	{
		D3D11_BUFFER_DESC BufferDesc{};//буферный ресурс
		BufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;//как буфер привязан к графическому конвейеру (сейчас - как буфер вершин на этапе IA)
		BufferDesc.ByteWidth = static_cast<UINT>(sizeof(SVertex3D) * m_Object3DData.vVertices.size());//размер буфера в байтах
		BufferDesc.CPUAccessFlags = 0;//флаг доступа к ЦП (0 - доступ не требуется)
		BufferDesc.MiscFlags = 0;//другие флаги
		BufferDesc.StructureByteStride = 0;//размер элемента вструктуре буфера в байтах (0 - неструктурированный буфер)
		BufferDesc.Usage = D3D11_USAGE_DEFAULT;//как будет происходить чтение и запись (сейчас - только от GPU)

		D3D11_SUBRESOURCE_DATA SubresourceData{};//структура для инициализации подресурса
		SubresourceData.pSysMem = &m_Object3DData.vVertices[0];//указатель на данные инициализации
		m_PtrDevice->CreateBuffer(&BufferDesc, &SubresourceData, &m_VertexBuffer);//создаем буфер
	}
	
	{
		D3D11_BUFFER_DESC BufferDesc{};//буферный ресурс
		BufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;//как буфер привязан к графическому конвейеру (сейчас - как индексный буфер на этапе IA)
		BufferDesc.ByteWidth = static_cast<UINT>(sizeof(STriangle) * m_Object3DData.vTriangles.size());//размер буфера в байтах
		BufferDesc.CPUAccessFlags = 0;//флаг доступа к ЦП (0 - доступ не требуется)
		BufferDesc.MiscFlags = 0;//другие флаги
		BufferDesc.StructureByteStride = 0;//размер элемента вструктуре буфера в байтах (0 - неструктурированный буфер)
		BufferDesc.Usage = D3D11_USAGE_DEFAULT;//как будет происходить чтение и запись (сейчас - только от GPU)

		D3D11_SUBRESOURCE_DATA SubresourceData{};//структура для инициализации подресурса
		SubresourceData.pSysMem = &m_Object3DData.vTriangles[0];//указатель на данные инициализации
		m_PtrDevice->CreateBuffer(&BufferDesc, &SubresourceData, &m_IndexBuffer);//создаем буфер
	}
}

void CObject3D::Draw()
{
	//привязка индексного буфера к этапу IA
	m_PtrDeviceContext->IASetIndexBuffer(m_IndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
	//привязка вертексного буфера к этапу IA
	m_PtrDeviceContext->IASetVertexBuffers(0, 1, m_VertexBuffer.GetAddressOf(), &m_VertexBufferStride, &m_VertexBufferOffset);
	//указываем информацию о типе примитива и порядке данных которые описывают входные данные этапа IA (что это полигоны)
	m_PtrDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	//Рисуем индексированные примитивы
	m_PtrDeviceContext->DrawIndexed(static_cast<UINT>(m_Object3DData.vTriangles.size() * 3), 0, 0);
}