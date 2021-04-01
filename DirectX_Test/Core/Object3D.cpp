#include "Object3D.h"
#include "GameWindow.h"

void CObject3D::Create(const SMesh& Mesh, const SMaterial& Material)
{
	vector<SMesh> vMeshes{ Mesh };
	vector<SMaterial> vMaterials{ Material };

	Create(vMeshes, vMaterials);
}

void CObject3D::Create(const vector<SMesh>& vMeshes, const vector<SMaterial>& vMaterials)
{
	m_vMeshes = vMeshes;
	m_vMaterials = vMaterials;

	m_vMeshBuffers.resize(m_vMeshes.size());
	for (size_t iMesh = 0; iMesh < m_vMeshes.size(); ++iMesh)
	{
		CreateMeshBuffers(iMesh);
	}
}

void CObject3D::CreateMeshBuffers(size_t MeshIndex)
{
	const SMesh& Mesh{ m_vMeshes[MeshIndex] };//Извлекаем адрес меша из массива
	
	{
		D3D11_BUFFER_DESC BufferDesc{};//буферный ресурс
		BufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;//как буфер привязан к графическому конвейеру (сейчас - как буфер вершин на этапе IA)
		BufferDesc.ByteWidth = static_cast<UINT>(sizeof(SVertex3D) * Mesh.vVertices.size());//размер буфера в байтах
		BufferDesc.CPUAccessFlags = 0;//флаг доступа к ЦП (0 - доступ не требуется)
		BufferDesc.MiscFlags = 0;//другие флаги
		BufferDesc.StructureByteStride = 0;//размер элемента вструктуре буфера в байтах (0 - неструктурированный буфер)
		BufferDesc.Usage = D3D11_USAGE_DEFAULT;//как будет происходить чтение и запись (сейчас - только от GPU)

		D3D11_SUBRESOURCE_DATA SubresourceData{};//структура для инициализации подресурса
		SubresourceData.pSysMem = &Mesh.vVertices[0];//указатель на данные инициализации
		m_PtrDevice->CreateBuffer(&BufferDesc, &SubresourceData, &m_vMeshBuffers[MeshIndex].VertexBuffer);//создаем буфер
	}
	
	{
		D3D11_BUFFER_DESC BufferDesc{};//буферный ресурс
		BufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;//как буфер привязан к графическому конвейеру (сейчас - как индексный буфер на этапе IA)
		BufferDesc.ByteWidth = static_cast<UINT>(sizeof(STriangle) * Mesh.vTriangles.size());//размер буфера в байтах
		BufferDesc.CPUAccessFlags = 0;//флаг доступа к ЦП (0 - доступ не требуется)
		BufferDesc.MiscFlags = 0;//другие флаги
		BufferDesc.StructureByteStride = 0;//размер элемента вструктуре буфера в байтах (0 - неструктурированный буфер)
		BufferDesc.Usage = D3D11_USAGE_DEFAULT;//как будет происходить чтение и запись (сейчас - только от GPU)

		D3D11_SUBRESOURCE_DATA SubresourceData{};//структура для инициализации подресурса
		SubresourceData.pSysMem = &Mesh.vTriangles[0];//указатель на данные инициализации
		m_PtrDevice->CreateBuffer(&BufferDesc, &SubresourceData, &m_vMeshBuffers[MeshIndex].IndexBuffer);//создаем буфер
	}
}

void CObject3D::Draw() const
{
	for (size_t iMesh = 0; iMesh < m_vMeshes.size(); ++iMesh)
	{
		const SMesh& Mesh{ m_vMeshes[iMesh] };

		m_PtrGameWindow->UpdateCBPSBaseMaterial(m_vMaterials[Mesh.MaterialID]);

		m_PtrDeviceContext->IASetIndexBuffer(m_vMeshBuffers[iMesh].IndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);//привязка индексного буфера к этапу IA
		m_PtrDeviceContext->IASetVertexBuffers(0, 1, m_vMeshBuffers[iMesh].VertexBuffer.GetAddressOf(),//привязка вертексного буфера к этапу IA
			&m_vMeshBuffers[iMesh].VertexBufferStride, &m_vMeshBuffers[iMesh].VertexBufferOffset);
		m_PtrDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);//указываем информацию о типе примитива и порядке данных которые описывают входные данные этапа IA (что это полигоны)

		m_PtrDeviceContext->DrawIndexed(static_cast<UINT>(Mesh.vTriangles.size() * 3), 0, 0);//Рисуем индексированные примитивы
	}
}

void CObject3D::DrawNormals() const
{
	for (size_t iMesh = 0; iMesh < m_vMeshes.size(); ++iMesh)
	{
		const SMesh& Mesh{ m_vMeshes[iMesh] };

		m_PtrDeviceContext->IASetVertexBuffers(0, 1, m_vMeshBuffers[iMesh].VertexBuffer.GetAddressOf(),//привязка вертексного буфера к этапу IA
			&m_vMeshBuffers[iMesh].VertexBufferStride, &m_vMeshBuffers[iMesh].VertexBufferOffset);
		m_PtrDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP);//указываем информацию о типе примитива и порядке данных которые описывают входные данные этапа IA (что это линии)

		m_PtrDeviceContext->Draw(static_cast<UINT>(Mesh.vVertices.size() * 2), 0);//Рисуем примитивы
	}
}