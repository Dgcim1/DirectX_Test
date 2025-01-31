#include "Object3D.h"
#include "GameWindow.h"
#include <limits>
#include <tuple>

static string SerializeXMMATRIX(const XMMATRIX& Matrix)
{
	string Result{};

	for (int iRow = 0; iRow < 4; ++iRow)
	{
		Result += '\t';
		for (int iCol = 0; iCol < 4; ++iCol)
		{
			Result += "[" + to_string(Matrix.r[iRow].m128_f32[iCol]) + "]";
		}
		Result += '\n';
	}
	Result += '\n';

	return Result;
}

void CObject3D::Create(const SMesh& Mesh, const SMaterial& Material)
{
	vector<SMesh> vMeshes{ Mesh };
	vector<SMaterial> vMaterials{ Material };

	Create(vMeshes, vMaterials);
}

void CObject3D::Create(const vector<SMesh>& vMeshes, const vector<SMaterial>& vMaterials)
{
	SModel Model{ vMeshes, vMaterials };

	Create(Model);
}

void CObject3D::Create(const SModel& Model)
{
	m_Model = Model;

	m_vMeshBuffers.resize(m_Model.vMeshes.size());
	for (size_t iMesh = 0; iMesh < m_Model.vMeshes.size(); ++iMesh)
	{
		CreateMeshBuffers(iMesh, m_Model.bIsAnimated);
	}

	m_vEmbeddedTextures.reserve(m_Model.vMaterials.size());
	for (SMaterial& Material : m_Model.vMaterials)
	{
		if (Material.bHasTexture && Material.vEmbeddedTextureRawData.size())
		{
			m_vEmbeddedTextures.emplace_back(make_unique<CTexture>(m_PtrDevice, m_PtrDeviceContext));

			m_vEmbeddedTextures.back()->CreateWICFromMemory(Material.vEmbeddedTextureRawData);

			Material.vEmbeddedTextureRawData.clear();
		}
		else if (Material.bHasTexture) 
		{
			m_vEmbeddedTextures.emplace_back(make_unique<CTexture>(m_PtrDevice, m_PtrDeviceContext));

			m_vEmbeddedTextures.back()->CreateFromFile(std::wstring(Material.TextureFileName.begin(), Material.TextureFileName.end()));

			Material.bHasEmbeddedTexture = true;
		}
	}
}

void CObject3D::CreateMeshBuffers(size_t MeshIndex, bool IsAnimated)
{
	const SMesh& Mesh{ m_Model.vMeshes[MeshIndex] };//Извлекаем адрес меша из массива
	
	{
		D3D11_BUFFER_DESC BufferDesc{};//буферный ресурс
		BufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;//как буфер привязан к графическому конвейеру (сейчас - как буфер вершин на этапе IA)
		BufferDesc.ByteWidth = static_cast<UINT>(sizeof(SVertex3D) * Mesh.vVertices.size());//размер буфера в байтах
		BufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;//флаг доступа к ЦП (0 - доступ не требуется)
		BufferDesc.MiscFlags = 0;//другие флаги
		BufferDesc.StructureByteStride = 0;//размер элемента вструктуре буфера в байтах (0 - неструктурированный буфер)
		BufferDesc.Usage = D3D11_USAGE_DYNAMIC;//как будет происходить чтение и запись (сейчас - только от GPU)

		D3D11_SUBRESOURCE_DATA SubresourceData{};//структура для инициализации подресурса
		SubresourceData.pSysMem = &Mesh.vVertices[0];//указатель на данные инициализации
		m_PtrDevice->CreateBuffer(&BufferDesc, &SubresourceData, &m_vMeshBuffers[MeshIndex].VertexBuffer);//создаем буфер
	}

	if (IsAnimated)
	{
		D3D11_BUFFER_DESC BufferDesc{};
		BufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		BufferDesc.ByteWidth = static_cast<UINT>(sizeof(SVertexAnimation) * Mesh.vVerticesAnimation.size());
		BufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		BufferDesc.MiscFlags = 0;
		BufferDesc.StructureByteStride = 0;
		BufferDesc.Usage = D3D11_USAGE_DYNAMIC;

		D3D11_SUBRESOURCE_DATA SubresourceData{};
		SubresourceData.pSysMem = &Mesh.vVerticesAnimation[0];
		m_PtrDevice->CreateBuffer(&BufferDesc, &SubresourceData, &m_vMeshBuffers[MeshIndex].VertexBufferAnimation);
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

void CObject3D::UpdateQuadUV(const XMFLOAT2& UVOffset, const XMFLOAT2& UVSize)
{
	float U0{ UVOffset.x };
	float V0{ UVOffset.y };
	float U1{ U0 + UVSize.x };
	float V1{ V0 + UVSize.y };

	m_Model.vMeshes[0].vVertices[0].TexCoord = XMVectorSet(U0, V0, 0, 0);
	m_Model.vMeshes[0].vVertices[1].TexCoord = XMVectorSet(U1, V0, 0, 0);
	m_Model.vMeshes[0].vVertices[2].TexCoord = XMVectorSet(U0, V1, 0, 0);
	m_Model.vMeshes[0].vVertices[3].TexCoord = XMVectorSet(U1, V1, 0, 0);

	UpdateMeshBuffer();
}

void CObject3D::UpdateMeshBuffer(size_t MeshIndex)
{
	D3D11_MAPPED_SUBRESOURCE MappedSubresource{};
	if (SUCCEEDED(m_PtrDeviceContext->Map(m_vMeshBuffers[MeshIndex].VertexBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedSubresource)))
	{
		memcpy(MappedSubresource.pData, &m_Model.vMeshes[MeshIndex].vVertices[0], sizeof(SVertex3D) * m_Model.vMeshes[MeshIndex].vVertices.size());

		m_PtrDeviceContext->Unmap(m_vMeshBuffers[MeshIndex].VertexBuffer.Get(), 0);
	}
}

std::tuple<XMFLOAT3, XMFLOAT3> CObject3D::GetBoxObject()
{
	XMFLOAT3 Point1{ -FLT_MAX , -FLT_MAX , -FLT_MAX };// 1 < 2
	XMFLOAT3 Point2{ FLT_MAX , FLT_MAX , FLT_MAX };
	for (auto& Mesh : m_Model.vMeshes)
	{
		for (auto& Vertex : Mesh.vVertices)
		{
			XMFLOAT4 vertex;
			XMStoreFloat4(&vertex, Vertex.Position);
			if (Point1.x < vertex.x) Point1.x = vertex.x;
			if (Point2.x > vertex.x) Point2.x = vertex.x;
			if (Point1.y < vertex.y) Point1.y = vertex.y;
			if (Point2.y > vertex.y) Point2.y = vertex.y;
			if (Point1.z < vertex.z) Point1.z = vertex.z;
			if (Point2.z > vertex.z) Point2.z = vertex.z;
		}
	}
	return std::make_tuple(Point1, Point2);
}

void CObject3D::Animate()
{
	if (!m_Model.vAnimations.size()) return;

	SModelAnimation& CurrentAnimation{ m_Model.vAnimations[m_CurrentAnimationIndex] };

	m_CurrentAnimationTick += CurrentAnimation.TicksPerSecond * 0.001f; // TEMPORARY SLOW DOWN!!
	if (m_CurrentAnimationTick >= CurrentAnimation.Duration)
	{
		m_CurrentAnimationTick = 0.0f;
	}

	CalculateAnimatedBoneMatrices(m_Model.vNodes[0], XMMatrixIdentity());

	memcpy(m_PtrGameWindow->cbVSAnimationBonesData.BoneMatrices, m_AnimatedBoneMatrices, sizeof(SCBVSAnimationBonesData));
	m_PtrGameWindow->m_VSAnimation->UpdateConstantBuffer(1);
}

void CObject3D::CalculateAnimatedBoneMatrices(const SModelNode& Node, XMMATRIX ParentTransform)
{
	XMMATRIX MatrixTransformation{ Node.MatrixTransformation * ParentTransform };

	if (Node.bIsBone)
	{
		SModelAnimation& CurrentAnimation{ m_Model.vAnimations[m_CurrentAnimationIndex] };
		if (CurrentAnimation.vNodeAnimations.size())
		{
			if (CurrentAnimation.mapNodeAnimationNameToIndex.find(Node.Name) != CurrentAnimation.mapNodeAnimationNameToIndex.end())
			{
				size_t NodeAnimationIndex{ CurrentAnimation.mapNodeAnimationNameToIndex[Node.Name] };

				SModelNodeAnimation& NodeAnimation{ CurrentAnimation.vNodeAnimations[NodeAnimationIndex] };

				XMMATRIX MatrixPosition{ XMMatrixIdentity() };
				XMMATRIX MatrixRotation{ XMMatrixIdentity() };
				XMMATRIX MatrixScaling{ XMMatrixIdentity() };

				{
					vector<SModelNodeAnimation::SKey>& vKeys{ NodeAnimation.vPositionKeys };
					SModelNodeAnimation::SKey KeyA{};
					SModelNodeAnimation::SKey KeyB{};
					for (uint32_t iKey = 0; iKey < (uint32_t)vKeys.size(); ++iKey)
					{
						if (vKeys[iKey].Time <= m_CurrentAnimationTick)
						{
							KeyA = vKeys[iKey];
							KeyB = vKeys[(iKey < (uint32_t)vKeys.size() - 1) ? iKey + 1 : 0];
						}
					}

					MatrixPosition = XMMatrixTranslationFromVector(KeyA.Value);
				}

				{
					vector<SModelNodeAnimation::SKey>& vKeys{ NodeAnimation.vRotationKeys };
					SModelNodeAnimation::SKey KeyA{};
					SModelNodeAnimation::SKey KeyB{};
					for (uint32_t iKey = 0; iKey < (uint32_t)vKeys.size(); ++iKey)
					{
						if (vKeys[iKey].Time <= m_CurrentAnimationTick)
						{
							KeyA = vKeys[iKey];
							KeyB = vKeys[(iKey < (uint32_t)vKeys.size() - 1) ? iKey + 1 : 0];
						}
					}

					MatrixRotation = XMMatrixRotationQuaternion(KeyA.Value);
				}

				{
					vector<SModelNodeAnimation::SKey>& vKeys{ NodeAnimation.vScalingKeys };
					SModelNodeAnimation::SKey KeyA{};
					SModelNodeAnimation::SKey KeyB{};
					for (uint32_t iKey = 0; iKey < (uint32_t)vKeys.size(); ++iKey)
					{
						if (vKeys[iKey].Time <= m_CurrentAnimationTick)
						{
							KeyA = vKeys[iKey];
							KeyB = vKeys[(iKey < (uint32_t)vKeys.size() - 1) ? iKey + 1 : 0];
						}
					}

					MatrixScaling = XMMatrixScalingFromVector(KeyA.Value);
				}

				MatrixTransformation = MatrixScaling * MatrixRotation * MatrixPosition * ParentTransform;
			}
		}

		// Transpose at the last moment!
		m_AnimatedBoneMatrices[Node.BoneIndex] = XMMatrixTranspose(Node.MatrixBoneOffset * MatrixTransformation);
	}

	if (Node.vChildNodeIndices.size())
	{
		for (auto iChild : Node.vChildNodeIndices)
		{
			CalculateAnimatedBoneMatrices(m_Model.vNodes[iChild], MatrixTransformation);
		}
	}
}

void CObject3D::Draw() const
{
	for (size_t iMesh = 0; iMesh < m_Model.vMeshes.size(); ++iMesh)
	{
		const SMesh& Mesh{ m_Model.vMeshes[iMesh] };
		const SMaterial& Material{ m_Model.vMaterials[Mesh.MaterialID] };

		// обновляем константный буфер пиксельного шейдера материала
		m_PtrGameWindow->cbPSBaseMaterialData.MaterialAmbient = Material.MaterialAmbient;
		m_PtrGameWindow->cbPSBaseMaterialData.MaterialDiffuse = Material.MaterialDiffuse;
		m_PtrGameWindow->cbPSBaseMaterialData.MaterialSpecular = Material.MaterialSpecular;
		m_PtrGameWindow->cbPSBaseMaterialData.SpecularExponent = Material.SpecularExponent;
		m_PtrGameWindow->cbPSBaseMaterialData.SpecularIntensity = Material.SpecularIntensity;
		m_PtrGameWindow->m_PSBase->UpdateConstantBuffer(2);

		if (Material.bHasTexture && Material.bHasEmbeddedTexture)
		{
			m_PtrGameWindow->cbPSBaseFlagsData.bUseTexture = TRUE;
			m_PtrGameWindow->m_PSBase->UpdateConstantBuffer(0);

			m_vEmbeddedTextures[Mesh.MaterialID]->Use();
		}

		m_PtrDeviceContext->IASetIndexBuffer(m_vMeshBuffers[iMesh].IndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);//привязка индексного буфера к этапу IA
		m_PtrDeviceContext->IASetVertexBuffers(0, 1, m_vMeshBuffers[iMesh].VertexBuffer.GetAddressOf(),//привязка вертексного буфера к этапу IA
			&m_vMeshBuffers[iMesh].VertexBufferStride, &m_vMeshBuffers[iMesh].VertexBufferOffset);
		
		if (m_Model.bIsAnimated)
		{
			m_PtrDeviceContext->IASetVertexBuffers(1, 1, m_vMeshBuffers[iMesh].VertexBufferAnimation.GetAddressOf(),
				&m_vMeshBuffers[iMesh].VertexBufferAnimationStride, &m_vMeshBuffers[iMesh].VertexBufferAnimationOffset);
		}

		m_PtrDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);//указываем информацию о типе примитива и порядке данных которые описывают входные данные этапа IA (что это полигоны)

		m_PtrDeviceContext->DrawIndexed(static_cast<UINT>(Mesh.vTriangles.size() * 3), 0, 0);//Рисуем индексированные примитивы
	}
}

void CObject3D::DrawNormals() const
{
	for (size_t iMesh = 0; iMesh < m_Model.vMeshes.size(); ++iMesh)
	{
		const SMesh& Mesh{ m_Model.vMeshes[iMesh] };

		m_PtrDeviceContext->IASetVertexBuffers(0, 1, m_vMeshBuffers[iMesh].VertexBuffer.GetAddressOf(),//привязка вертексного буфера к этапу IA
			&m_vMeshBuffers[iMesh].VertexBufferStride, &m_vMeshBuffers[iMesh].VertexBufferOffset);
		
		if (m_Model.bIsAnimated)
		{
			m_PtrDeviceContext->IASetVertexBuffers(1, 1, m_vMeshBuffers[iMesh].VertexBufferAnimation.GetAddressOf(),
				&m_vMeshBuffers[iMesh].VertexBufferAnimationStride, &m_vMeshBuffers[iMesh].VertexBufferAnimationOffset);
		}
		
		m_PtrDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP);//указываем информацию о типе примитива и порядке данных которые описывают входные данные этапа IA (что это линии)

		m_PtrDeviceContext->Draw(static_cast<UINT>(Mesh.vVertices.size() * 2), 0);//Рисуем примитивы
	}
}