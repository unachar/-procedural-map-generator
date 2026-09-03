#include "main.h"
#include "renderer.h"
#include "meshField.h"
#include "perlinnoise.h"
#include "texture.h"
#include "manager.h"
#include "scene.h"
#include "player.h"
#include "ImGui/imgui.h"
#include "chunk.h"
#include "input.h"

#include <vector>
#include <algorithm>
//ImGuiパラメーター
float MeshField::s_NoiseScale = m_kNoiseScaleBase;
float MeshField::s_NoiseOctaves = m_kNoiseOctavesBase;
float MeshField::s_NoisePersistence = m_kNoisePersistenceBase;
float MeshField::s_HeightScale = m_kHeightScaleBase;
int MeshField::s_Seed = m_kSeedBase;


//===================================================

int MeshField::s_NextSeed = m_kSeedBase;

void MeshField::Initialize()
{
	if (s_NextSeed != 0)
	{
		m_Seed = s_NextSeed;
		s_Seed = s_NextSeed;
		s_NextSeed = 0;
	}
	
	PerlinNoise noise(s_Seed);
	// 頂点バッファ生成
	{
		for (int x = 0; x < m_kMapSizeX; x++)
		{
			for (int z = 0; z < m_kMapSizeZ; z++)
			{
				// ワールド座標を計算 (ローカル座標 + ワールド位置)
				float worldX = m_WorldPosition.x + (x - m_kMapSizeX/2) * 5.0f;
				float worldZ = m_WorldPosition.z + (z - m_kMapSizeZ/2) * - 5.0f;
				
				// ワールド座標をノイズ入力として使用
				float nx = worldX * s_NoiseScale * 0.01f;
				float nz = worldZ * s_NoiseScale * 0.01f;
				
				float y = noise.octaveNoise2D(nx, nz, (int)s_NoiseOctaves, s_NoisePersistence) * s_HeightScale;
				//float y = 0.f;

				m_Vertex[x][z].Position = XMFLOAT3((x - m_kMapSizeX/2) * 5.0f, y, (z - m_kMapSizeZ/2) * -5.0f);
				m_Vertex[x][z].TexCoord = XMFLOAT2((float)x / (float)(m_kMapSizeX - 1), (float)z / (float)(m_kMapSizeZ - 1));
				m_Vertex[x][z].Diffuse = XMFLOAT4(1.f, 1.f, 1.f, 1.f);
				
				m_Vertex[x][z].Normal = XMFLOAT3(0.0f, 1.0f, 0.0f);

				float alphaThreshold = -10.f;
				float alpha; 
				if (y < alphaThreshold)
				{
					alpha = 0.f;
				}
				else
				{
					alpha = 1.f;
				}
				m_Vertex[x][z].Diffuse = XMFLOAT4(1.f, 1.f, 1.f, alpha);
			}
		}

		for (int x = 1; x < m_kMapSizeX - 1; x++)
		{
			for (int z = 1; z < m_kMapSizeZ - 1; z++)
			{
				Vector3 vx, vz, vn;
				vx.x = m_Vertex[x + 1][z].Position.x - m_Vertex[x - 1][z].Position.x;
				vx.y = m_Vertex[x + 1][z].Position.y - m_Vertex[x - 1][z].Position.y;
				vx.z = m_Vertex[x + 1][z].Position.z - m_Vertex[x - 1][z].Position.z;

				vz.x = m_Vertex[x][z - 1].Position.x - m_Vertex[x][z + 1].Position.x;
				vz.y = m_Vertex[x][z - 1].Position.y - m_Vertex[x][z + 1].Position.y;
				vz.z = m_Vertex[x][z - 1].Position.z - m_Vertex[x][z + 1].Position.z;

				vn = Vector3::cross(vz, vx);
				vn.Normalize();

				m_Vertex[x][z].Normal.x = vn.x;
				m_Vertex[x][z].Normal.y = vn.y;
				m_Vertex[x][z].Normal.z = vn.z;
			}
		}

		


		D3D11_BUFFER_DESC bd;
		ZeroMemory(&bd, sizeof(bd));
		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.ByteWidth = sizeof(VERTEX_3D) * m_kMapSizeX * m_kMapSizeZ;
		bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bd.CPUAccessFlags = 0;

		D3D11_SUBRESOURCE_DATA sd;
		ZeroMemory(&sd, sizeof(sd));
		sd.pSysMem = m_Vertex;

		Renderer::GetDevice()->CreateBuffer(&bd, &sd, &m_VertexBuffer);


	}


	// インデックスバッファ生成
	{
		unsigned int index[((m_kMapSizeX + 1) * 2) * (m_kMapSizeX - 1) - 2 ];

		int i = 0;
		for (int x = 0; x < m_kMapSizeX - 1; x++)
		{
			for (int z = 0; z < m_kMapSizeZ; z++)
			{
				index[i] = x * m_kMapSizeZ + z;
				i++;

				index[i] = (x + 1) * m_kMapSizeZ + z;
				i++;
			}

			if (x == m_kMapSizeX - 2)
				break;

			index[i] = (x + 1) * m_kMapSizeZ + (m_kMapSizeZ - 1);
			i++;

			index[i] = (x + 1) * m_kMapSizeZ;
			i++;
		}

		D3D11_BUFFER_DESC bd;
		ZeroMemory(&bd, sizeof(bd));
		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.ByteWidth = sizeof(unsigned int) * (((m_kMapSizeX + 1) * 2) * (m_kMapSizeX - 1) - 2);
		bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
		bd.CPUAccessFlags = 0;

		D3D11_SUBRESOURCE_DATA sd;
		ZeroMemory(&sd, sizeof(sd));
		sd.pSysMem = index;

		Renderer::GetDevice()->CreateBuffer(&bd, &sd, &m_IndexBuffer);
	}



	// テクスチャ読み込み
	m_Texture = Texture::Load("asset\\texture\\field.jpg");
	m_Texture_dirt = Texture::Load("asset\\texture\\dirt.jpg");

	Renderer::CreateVertexShader(
		&m_VertexShader,
		&m_VertexLayout,
		"shader\\fieldVs.cso"
	);


	Renderer::CreatePixelShader(
		&m_PixelShader,
		"shader\\fieldPS.cso"
	);


	// グリッド用ラスタライザーステート作成
	D3D11_RASTERIZER_DESC rasterDesc = {};
	rasterDesc.FillMode = D3D11_FILL_SOLID;
	rasterDesc.CullMode = D3D11_CULL_NONE;
	rasterDesc.FrontCounterClockwise = FALSE;
	rasterDesc.DepthBias = 0;
	rasterDesc.SlopeScaledDepthBias = 0.0f;
	rasterDesc.DepthBiasClamp = 0.0f;
	rasterDesc.DepthClipEnable = TRUE;
	rasterDesc.ScissorEnable = FALSE;
	rasterDesc.MultisampleEnable = FALSE;
	rasterDesc.AntialiasedLineEnable = TRUE;
	Renderer::GetDevice()->CreateRasterizerState(&rasterDesc, &m_GridRasterState);
}


void MeshField::Finalize()
{
	if (m_VertexBuffer) m_VertexBuffer->Release();
	if (m_IndexBuffer) m_IndexBuffer->Release();
	if (m_PixelShader) m_PixelShader->Release();
	if (m_VertexShader) m_VertexShader->Release();
	if (m_VertexLayout) m_VertexLayout->Release();

	if (m_GridVertexBuffer) m_GridVertexBuffer->Release();
	if (m_GridIndexBuffer) m_GridIndexBuffer->Release();
	if (m_GridRasterState) m_GridRasterState->Release();
}


void MeshField::Update()
{
	
}


void MeshField::Draw()
{
	Renderer::GetDeviceContext()->IASetInputLayout(m_VertexLayout);
	Renderer::GetDeviceContext()->VSSetShader(m_VertexShader, NULL, 0);
	Renderer::GetDeviceContext()->PSSetShader(m_PixelShader, NULL, 0);

	Renderer::AddWorldMatrix(
		{ m_Scale.x, m_Scale.y, m_Scale.z },
		{ m_Rotation.x, m_Rotation.y, m_Rotation.z },
		{ m_Position.x, m_Position.y, m_Position.z }
	);

	// 頂点バッファ設定
	UINT stride = sizeof( VERTEX_3D );
	UINT offset = 0;
	Renderer::GetDeviceContext()->IASetVertexBuffers( 0, 1, &m_VertexBuffer, &stride, &offset );

	// インデックスバッファ設定
	Renderer::GetDeviceContext()->IASetIndexBuffer( 
		m_IndexBuffer, DXGI_FORMAT_R32_UINT, 0 );

	// マテリアル設定
	MATERIAL material;
	ZeroMemory( &material, sizeof(material) );
	material.Diffuse = XMFLOAT4( 1.0f, 1.0f, 1.0f, 1.0f );
	material.TextureEnable = true;
	Renderer::SetMaterial( material );

	// テクスチャ設定
	Renderer::GetDeviceContext()->PSSetShaderResources(0, 1, &m_Texture);
	Renderer::GetDeviceContext()->PSSetShaderResources(1, 1, &m_Texture_dirt);

	// プリミティブトポロジ設定
	Renderer::GetDeviceContext()->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP );
	
	// ポリゴン描画
	Renderer::GetDeviceContext()->DrawIndexed(
		((m_kMapSizeX + 1) * 2) * (m_kMapSizeX - 1) - 2, 0, 0);

	Renderer::AddDrawCall();

	Renderer::GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

	DrawGrid();
}

float MeshField::GetHeight(Vector3 position) const
{
	const float gridSpacing = 5.0f;
	const float halfW = m_kMapSizeX / 2.0f;
	const float halfH = m_kMapSizeZ / 2.0f;

	float localX = (position.x - m_Position.x) / gridSpacing + halfW;
	float localZ = -(position.z - m_Position.z) / gridSpacing + halfH;

	int x0 = clamp((int)floorf(localX), 0, m_kMapSizeX - 1);
	int z0 = clamp((int)floorf(localZ), 0, m_kMapSizeZ - 1);
	int x1 = min(x0 + 1, m_kMapSizeX - 1);
	int z1 = min(z0 + 1, m_kMapSizeZ - 1);

	float tx = localX - (float)x0;
	float tz = localZ - (float)z0;

	float h00 = m_Vertex[x0][z0].Position.y;
	float h10 = m_Vertex[x1][z0].Position.y;
	float h01 = m_Vertex[x0][z1].Position.y;
	float h11 = m_Vertex[x1][z1].Position.y;

	float h0 = h00 * (1 - tx) + h10 * tx;
	float h1 = h01 * (1 - tx) + h11 * tx;
	return h0 * (1 - tz) + h1 * tz;
}

void MeshField::SetHeightAtPosition(Vector3 position, float height)
{
	float fx = (position.x / 5.f) + 10.f;
	float fz = (position.z / -5.f) + 10.f;

	const int radius = 2;
	
	for (int dx = -radius; dx <= radius; dx++)
	{
		for (int dz = -radius; dz <= radius; dz++)
		{
			int x = (int)fx + dx;
			int z = (int)fz + dz;
			
			if (x < 0 || x >= m_kMapSizeX || z < 0 || z >= m_kMapSizeZ) continue;
			
			// 距離に基づく重み計算
			float distance = sqrtf((float)(dx * dx + dz * dz));
			float weight = 1.0f - (distance / (float)(radius + 1));
			weight = max(0.0f, weight);
			
			float currentHeight = m_Vertex[x][z].Position.y;
			float newHeight = currentHeight * (1.0f - weight * 0.5f) + height * weight * 0.5f;
			
			m_Vertex[x][z].Position.y = newHeight;
		}
	}
	
	// 頂点バッファを更新
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	Renderer::GetDeviceContext()->Map(m_VertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	memcpy(mappedResource.pData, m_Vertex, sizeof(VERTEX_3D) * m_kMapSizeX * m_kMapSizeZ);
	Renderer::GetDeviceContext()->Unmap(m_VertexBuffer, 0);
}

void MeshField::DrawGrid()
{
	if (!m_GridRasterState) return;

	// グリッド線用の頂点を生成 
	if (m_NeedUpdateGrid || !m_GridVertexBuffer)
	{
		if (m_GridVertexBuffer) { m_GridVertexBuffer->Release(); m_GridVertexBuffer = nullptr; }
		if (m_GridIndexBuffer) { m_GridIndexBuffer->Release(); m_GridIndexBuffer = nullptr; }

		const int totalGridX = m_kMapSizeX * 2 - 1;
		const int totalGridZ = m_kMapSizeZ * 2 - 1;
		
		vector<VERTEX_3D> interpolatedVertices(totalGridX * totalGridZ);
		
		for (int x = 0; x < m_kMapSizeX; x++) 
		{
			for (int z = 0; z < m_kMapSizeZ; z++)
			{
				interpolatedVertices[(x * 2) * totalGridZ + (z * 2)] = m_Vertex[x][z];
			}
		}
		
		for (int x = 0; x < totalGridX; x++)
		{
			for (int z = 0; z < totalGridZ; z++)
			{
				if (x % 2 == 0 && z % 2 == 0) continue;
				
				int baseX = x / 2;
				int baseZ = z / 2;
				int nextX = (x % 2 == 1) ? baseX + 1 : baseX;
				int nextZ = (z % 2 == 1) ? baseZ + 1 : baseZ;
				
				if (nextX >= m_kMapSizeX) nextX = m_kMapSizeX - 1;
				if (nextZ >= m_kMapSizeZ) nextZ = m_kMapSizeZ - 1;
				
				XMVECTOR pos1 = XMLoadFloat3(&m_Vertex[baseX][baseZ].Position);
				XMVECTOR pos2 = XMLoadFloat3(&m_Vertex[nextX][nextZ].Position);
				XMVECTOR midPos = (pos1 + pos2) * 0.5f;
				XMStoreFloat3(&interpolatedVertices[x * totalGridZ + z].Position, midPos);
				
				interpolatedVertices[x * totalGridZ + z].TexCoord = XMFLOAT2(
					(m_Vertex[baseX][baseZ].TexCoord.x + m_Vertex[nextX][nextZ].TexCoord.x) * 0.5f,
					(m_Vertex[baseX][baseZ].TexCoord.y + m_Vertex[nextX][nextZ].TexCoord.y) * 0.5f
				);
				interpolatedVertices[x * totalGridZ + z].Diffuse = m_Vertex[baseX][baseZ].Diffuse;
				interpolatedVertices[x * totalGridZ + z].Normal = m_Vertex[baseX][baseZ].Normal;
			}
		}
		
		vector<unsigned int> gridIndices;
		gridIndices.reserve((totalGridX * (totalGridZ - 1) + totalGridZ * (totalGridX - 1)) * 2);

		// 横線
		for (int x = 0; x < totalGridX; x++) 
		{
			for (int z = 0; z < totalGridZ - 1; z++) 
			{
				gridIndices.push_back(x * totalGridZ + z);
				gridIndices.push_back(x * totalGridZ + (z + 1));
			}
		}

		// 縦線
		for (int z = 0; z < totalGridZ; z++) 
		{
			for (int x = 0; x < totalGridX - 1; x++)
			{
				gridIndices.push_back(x * totalGridZ + z);
				gridIndices.push_back((x + 1) * totalGridZ + z);
			}
		}

		m_GridIndexCount = (int)gridIndices.size();

		D3D11_BUFFER_DESC vertexBd = {};
		vertexBd.Usage = D3D11_USAGE_DEFAULT;
		vertexBd.ByteWidth = sizeof(VERTEX_3D) * (UINT)interpolatedVertices.size();
		vertexBd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		D3D11_SUBRESOURCE_DATA vertexSd = { interpolatedVertices.data() };
		Renderer::GetDevice()->CreateBuffer(&vertexBd, &vertexSd, &m_GridVertexBuffer);

		D3D11_BUFFER_DESC gridBd = {};
		gridBd.Usage = D3D11_USAGE_DEFAULT;
		gridBd.ByteWidth = sizeof(unsigned int) * m_GridIndexCount;
		gridBd.BindFlags = D3D11_BIND_INDEX_BUFFER;
		D3D11_SUBRESOURCE_DATA gridSd = { gridIndices.data() };
		Renderer::GetDevice()->CreateBuffer(&gridBd, &gridSd, &m_GridIndexBuffer);

		m_NeedUpdateGrid = false;
	}

	if (!m_GridVertexBuffer || !m_GridIndexBuffer) return;

	UINT stride = sizeof(VERTEX_3D);
	UINT offset = 0;
	Renderer::GetDeviceContext()->IASetVertexBuffers(0, 1, &m_GridVertexBuffer, &stride, &offset);
	Renderer::GetDeviceContext()->IASetIndexBuffer(m_GridIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

	MATERIAL gridMaterial = {};
	gridMaterial.Diffuse = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
	gridMaterial.TextureEnable = false;
	Renderer::SetMaterial(gridMaterial);
	
	ID3D11RasterizerState* originalRasterState = nullptr;
	Renderer::GetDeviceContext()->RSGetState(&originalRasterState);
	Renderer::GetDeviceContext()->RSSetState(m_GridRasterState);

	Renderer::GetDeviceContext()->DrawIndexed(m_GridIndexCount, 0, 0);

	Renderer::GetDeviceContext()->RSSetState(originalRasterState);
	if (originalRasterState) originalRasterState->Release();

	// 元の頂点バッファに戻す
	Renderer::GetDeviceContext()->IASetVertexBuffers(0, 1, &m_VertexBuffer, &stride, &offset);
}

void MeshField::DrawImGui()
{
	bool regenerate = false;

	ImGui::Separator();
	ImGui::Text("Terrain Parameters");
	
	if (ImGui::InputFloat("Noise Scale", &s_NoiseScale, m_kNoiseScaleMin, m_kNoiseScaleMax)) regenerate = true;
	if (ImGui::InputFloat("Octaves", &s_NoiseOctaves, 1.0f, 1.0f)) regenerate = true;
	if (ImGui::InputFloat("Persistence", &s_NoisePersistence, 0.01f, 0.1f)) regenerate = true;
	if (ImGui::InputFloat("Height Scale", &s_HeightScale, 0.1f, 1.0f)) regenerate = true;
	if (ImGui::InputInt("Seed", &s_Seed, 1, 100)) regenerate = true;
	if (regenerate) 
	{
		m_NeedRegenerate = true;
		m_NeedUpdateGrid = true;
	}
}

void MeshField::RegenerateTerrain()
{
	auto chunk = Manager::GetScene()->GetGameObject<Chunk>();
	if (chunk) 
	{
		for (auto field : chunk->m_ChildFields)
		{
			if (field)
			{
				field->m_Seed = s_Seed;
				
				if (field->m_VertexBuffer) field->m_VertexBuffer->Release();
				if (field->m_IndexBuffer) field->m_IndexBuffer->Release();
				
				field->m_NeedUpdateGrid = true;
				field->Initialize();
			}
		}
	}
}

