#include "manager.h"
#include "scene.h"
#include "chunk.h"
#include "tree.h"
#include "rock.h"
#include "player.h"
#include "camera.h"
#include "modelRenderer.h"
#include <random>
#include "ImGui/imgui.h"

// static メンバ変数の実体定義
bool Chunk::m_EnableFrustumCulling = true;
bool Chunk::m_EnableDistanceCulling = true;
bool Chunk::m_ShowDebugInfo = false;


void Chunk::Initialize()
{
	m_Seed = m_kDefaultSeed;

	m_pTree = new Tree();
	m_pTree->Initialize();
	m_pRock = new Rock();
	m_pRock->Initialize();

	m_ChunkOrigin.x = (m_kChunkNumX - 1) * m_Spacing / 2.f;
	m_ChunkOrigin.z = (m_kChunkNumZ - 1) * m_Spacing / 2.f;

	m_ChunkMargin = m_kChunkMarginBase;
	m_PosX = 0.f;
	m_PosZ = 0.f;
	m_PosY = 0.f;

	m_DrawnFieldCount = 0;
	m_DrawnTreeCount = 0;
	m_DrawnRockCount = 0;

	m_Spacing = m_kSpacingBase;
	m_HeightLimit = m_kHeightLimitBase;

	m_ViewDistance = m_kViewDistanceBase;

	for (int x = 0; x < m_kChunkNumX; x++)
	{
		for (int z = 0; z < m_kChunkNumZ; z++)
		{
			MeshField* field = new MeshField();
			field->SetSeed(m_Seed * (x * 5 + z));
			
			// チャンクの中心が(0,0,0)になるようにオフセットを計算
			float offsetX = (x - (m_kChunkNumX - 1) / 2.0f) * m_Spacing;
			float offsetZ = (z - (m_kChunkNumZ - 1) / 2.0f) * m_Spacing;

			Vector3 fieldPos = Vector3(offsetX, 0.f, offsetZ);
			field->SetPosition(fieldPos);
			field->SetWorldPosition(fieldPos);
			
			field->Initialize();

			m_ChildFields.push_back(field);
		}
	}

	m_FieldTrees.resize(m_ChildFields.size());
	m_FieldRocks.resize(m_ChildFields.size());


	m_PrevNoiseScale = MeshField::s_NoiseScale;
	m_PrevNoiseOctaves = MeshField::s_NoiseOctaves;
	m_PrevNoisePersistence = MeshField::s_NoisePersistence;
	m_PrevHeightScale = MeshField::s_HeightScale;
	m_PrevSpacing = m_Spacing;
	m_PrevHeightLimit = m_HeightLimit;
	m_PrevSeed = MeshField::s_Seed;

	GenerateTerrainObjects();
}

void Chunk::Finalize()
{
	m_FieldTrees.clear();
	m_FieldRocks.clear();

	for (auto field : m_ChildFields)
	{
		if (field)
		{
			field->Finalize();
			delete field;
		}
	}
	m_ChildFields.clear();


	if (m_pTree)
	{
		m_pTree->Finalize();
		delete m_pTree;
		m_pTree = nullptr;
	}

	if (m_pRock)
	{
		m_pRock->Finalize();
		delete m_pRock;
		m_pRock = nullptr;
	}
}

void Chunk::Update()
{
	UpdatePlayerPosition();

	// 値が変わっていたら全フィールドに更新フラグを立てる
	if (m_PrevNoiseScale != MeshField::s_NoiseScale ||
		m_PrevNoiseOctaves != MeshField::s_NoiseOctaves ||
		m_PrevNoisePersistence != MeshField::s_NoisePersistence ||
		m_PrevHeightScale != MeshField::s_HeightScale ||
		m_PrevSeed != MeshField::s_Seed ||
		m_PrevSpacing != m_Spacing ||
		m_PrevHeightLimit != m_HeightLimit)
	{

		if (m_PrevSpacing != m_Spacing)
		{
			for (int x = 0; x < m_kChunkNumX; x++)
			{
				for (int z = 0; z < m_kChunkNumZ; z++)
				{
					int fIdx = x * m_kChunkNumZ + z;
					MeshField* field = m_ChildFields[fIdx];
					if (field)
					{
						float offsetX = (x - (m_kChunkNumX - 1) / 2.0f) * m_Spacing;
						float offsetZ = (z - (m_kChunkNumZ - 1) / 2.0f) * m_Spacing;
						Vector3 fieldPos = Vector3(offsetX, 0.f, offsetZ);
						field->SetPosition(fieldPos);
						field->SetWorldPosition(fieldPos);
					}
				}
			}
		}

		for (auto field : m_ChildFields)
		{
			if (field) field->m_NeedRegenerate = true;
		}

		// オブジェクトの再生成
		for (auto& vt : m_FieldTrees) vt.clear();
		for (auto& vr : m_FieldRocks) vr.clear();
		GenerateTerrainObjects();


		m_PrevNoiseScale = MeshField::s_NoiseScale;
		m_PrevNoiseOctaves = MeshField::s_NoiseOctaves;
		m_PrevNoisePersistence = MeshField::s_NoisePersistence;
		m_PrevHeightScale = MeshField::s_HeightScale;
		m_PrevSpacing = m_Spacing;
		m_PrevHeightLimit = m_HeightLimit;
		m_PrevSeed = MeshField::s_Seed;
	}


	if (m_EnableFrustumCulling)
	{
		auto cam = Manager::GetScene()->GetGameObject<Camera>();
		if (cam)
		{
			Vector3 camPos = cam->GetPosition();
			Vector3 camTarget = cam->GetTarget();
			XMFLOAT3 up(0.f, 1.f, 0.f);

			XMMATRIX view = XMMatrixLookAtLH(
				XMVectorSet(camPos.x, camPos.y, camPos.z, 1.f),
				XMVectorSet(camTarget.x, camTarget.y, camTarget.z, 1.f),
				XMLoadFloat3(&up));

			float fovy = cam->GetFovy();
			if (fovy <= 0.0f) fovy = 1.0f;


			float farClip = m_EnableDistanceCulling ? m_ViewDistance : 5000.0f;
			if (farClip < 100.0f) farClip = 1000.0f;

			XMMATRIX proj = XMMatrixPerspectiveFovLH(
				fovy, (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT, 1.f, farClip);

			XMMATRIX vp = XMMatrixMultiply(view, proj);
			XMFLOAT4X4 m;
			XMStoreFloat4x4(&m, vp);

			// 6つの平面を抽出
			m_FrustumPlanes[0] = { m._14 + m._11, m._24 + m._21, m._34 + m._31, m._44 + m._41 }; // Left
			m_FrustumPlanes[1] = { m._14 - m._11, m._24 - m._21, m._34 - m._31, m._44 - m._41 }; // Right
			m_FrustumPlanes[2] = { m._14 + m._12, m._24 + m._22, m._34 + m._32, m._44 + m._42 }; // Bottom
			m_FrustumPlanes[3] = { m._14 - m._12, m._24 - m._22, m._34 - m._32, m._44 - m._42 }; // Top
			m_FrustumPlanes[4] = {         m._13,         m._23,         m._33,         m._43 }; // Near
			m_FrustumPlanes[5] = { m._14 - m._13, m._24 - m._23, m._34 - m._33, m._44 - m._43 }; // Far

			// 正規化
			for (int i = 0; i < m_kFrustumPlanesNum; i++)
			{
				float len = sqrtf(m_FrustumPlanes[i].x * m_FrustumPlanes[i].x + m_FrustumPlanes[i].y * m_FrustumPlanes[i].y + m_FrustumPlanes[i].z * m_FrustumPlanes[i].z);
				if (len > 0.0f)
				{
					m_FrustumPlanes[i].x /= len;
					m_FrustumPlanes[i].y /= len;
					m_FrustumPlanes[i].z /= len;
					m_FrustumPlanes[i].w /= len;
				}
			}
		}
	}


	int updateCount = 0;
	const int MAX_UPDATES_PER_FRAME = m_kMaxUpdateFrame;
	float panelRadius = m_Spacing * m_kSpacingOffset;

	vector<CullingResult> panelCulling(m_ChildFields.size(), CullingResult::Outside);

	for (size_t i = 0; i < m_ChildFields.size(); i++) 
	{
		MeshField* field = m_ChildFields[i];
		if (field) 
		{
			Vector3 worldPos = m_Position + field->GetPosition();
			if (IsInViewDistance(worldPos))
			{
				CullingResult res = CheckFrustum(worldPos, panelRadius);
				panelCulling[i] = res;

				if (res != CullingResult::Outside)
				{
					// 更新が必要で、かつフレーム制限内であれば再生成
					if (field->m_NeedRegenerate && updateCount < MAX_UPDATES_PER_FRAME)
					{
						if (field->GetVertexBuffer()) field->GetVertexBuffer()->Release();
						if (field->GetIndexBuffer()) field->GetIndexBuffer()->Release();

						field->Initialize();
						field->m_NeedRegenerate = false;
						updateCount++;
					}
					field->Update();
				}
			}
		}
	}

	if (updateCount > 0)
	{
		UpdateTerrainObjectPositions();
	}
}


void Chunk::Draw()
{
	Renderer::AddWorldMatrix(
		{ m_Scale.x,m_Scale.y,m_Scale.z },
		{ m_Rotation.x,m_Rotation.y,m_Rotation.z },
		{ m_Position.x,m_Position.y,m_Position.z }
	);


	m_DrawnFieldCount = 0;
	m_DrawnTreeCount = 0;
	m_DrawnRockCount = 0;

	XMMATRIX chunkS = XMMatrixScaling(m_Scale.x, m_Scale.y, m_Scale.z);
	XMMATRIX chunkR = XMMatrixRotationRollPitchYaw(m_Rotation.x, m_Rotation.y, m_Rotation.z);
	XMMATRIX chunkT = XMMatrixTranslation(m_Position.x, m_Position.y, m_Position.z);
	XMMATRIX chunkWorld = chunkS * chunkR * chunkT;

	float panelRadius = m_Spacing * m_kSpacingOffset;

	if (m_pTree) m_pTree->ClearInstances();
	if (m_pRock) m_pRock->ClearInstances();

	for (size_t i = 0; i < m_ChildFields.size(); i++)
	{
		MeshField* field = m_ChildFields[i];
		if (!field) continue;

		Vector3 worldPos = m_Position + field->GetPosition();
		if (IsInViewDistance(worldPos))
		{
			CullingResult res = CheckFrustum(worldPos, panelRadius);
			if (res != CullingResult::Outside)
			{
				// フィールド描画
				field->Draw();
				m_DrawnFieldCount++;

				// フィールド内のオブジェクト描画
				if (m_pTree)
				{
					for (auto& tree : m_FieldTrees[i])
					{
						bool shouldDraw = (res == CullingResult::Inside);
						if (!shouldDraw)
						{
							Vector3 tWorldPos = m_Position + tree.position;
							shouldDraw = (CheckFrustum(tWorldPos, 2.0f) != CullingResult::Outside);
						}

						if (shouldDraw)
						{
							m_pTree->AddInstance(tree.localWorld * chunkWorld);
							m_DrawnTreeCount++;
						}
					}
				}

				if (m_pRock)
				{
					for (auto& rock : m_FieldRocks[i])
					{
						bool shouldDraw = (res == CullingResult::Inside);
						if (!shouldDraw)
						{
							Vector3 rWorldPos = m_Position + rock.position;
							shouldDraw = (CheckFrustum(rWorldPos, 2.0f) != CullingResult::Outside);
						}

						if (shouldDraw)
						{
							m_pRock->AddInstance(rock.localWorld * chunkWorld);
							m_DrawnRockCount++;
						}
					}
				}
			}
		}
	}

	if (m_pTree)
	{
		m_pTree->UpdateInstanceBuffer();
		m_pTree->Draw();
	}

	if (m_pRock)
	{
		m_pRock->UpdateInstanceBuffer();
		m_pRock->Draw();
	}
}


void Chunk::GenerateTerrainObjects()
{
	random_device rd;
	mt19937 rng(m_Seed);
	
	// チャンク全体の範囲を取得
	float halfChunkSize = (m_kChunkNumX * m_Spacing) / 2.0f;
	uniform_real_distribution<float> distPos(-halfChunkSize, halfChunkSize);
	
	uniform_real_distribution<float> scaleDist(m_kRandScaleMin, m_kRandScaleMax);
	uniform_real_distribution<float> rotDist(0.f, XM_2PI);


	const int totalTrees = m_kChunkNumX * m_kChunkNumZ * m_kTreeNum;
	const int totalRocks = m_kChunkNumX * m_kChunkNumZ * m_kRockNum;

	// 木の生成
	for (int i = 0; i < totalTrees; i++)
	{
		float x = distPos(rng);
		float z = distPos(rng);
		Vector3 pos = { x, 0.f, z };


		if (!IsPositionInChunk(pos)) continue;

		float y = GetHeight(pos);
		if (y < m_HeightLimit * 2.0f)
		{
			// どのフィールドに属するかインデックスを計算
			int fx = (int)floorf((x / m_Spacing) + (m_kChunkNumX / 2.0f));
			int fz = (int)floorf((z / m_Spacing) + (m_kChunkNumZ / 2.0f));
			
			if (fx >= 0 && fx < m_kChunkNumX && fz >= 0 && fz < m_kChunkNumZ)
			{
				int fIdx = fx * m_kChunkNumZ + fz;
				
				TerrainObject obj{};
				obj.position = { x, y, z };
				float s = scaleDist(rng);
				obj.scale = { s, s, s };
				obj.rotation = { 0.f, rotDist(rng), 0.f };
				
				XMMATRIX S = XMMatrixScaling(obj.scale.x, obj.scale.y, obj.scale.z);
				XMMATRIX R = XMMatrixRotationRollPitchYaw(obj.rotation.x, obj.rotation.y, obj.rotation.z);
				XMMATRIX T = XMMatrixTranslation(obj.position.x, obj.position.y, obj.position.z);
				obj.localWorld = S * R * T;

				m_FieldTrees[fIdx].push_back(obj);
			}
		}
	}

	// 岩の生成
	for (int i = 0; i < totalRocks; i++)
	{
		float x = distPos(rng);
		float z = distPos(rng);
		Vector3 pos = { x, 0.f, z };

		if (!IsPositionInChunk(pos)) continue;

		float y = GetHeight(pos);
		if (y < m_HeightLimit * 2.0f)
		{
			int fx = (int)floorf((x / m_Spacing) + (m_kChunkNumX / 2.0f));
			int fz = (int)floorf((z / m_Spacing) + (m_kChunkNumZ / 2.0f));
			
			if (fx >= 0 && fx < m_kChunkNumX && fz >= 0 && fz < m_kChunkNumZ)
			{
				int fIdx = fx * m_kChunkNumZ + fz;

				TerrainObject obj{};
				obj.position = { x, y, z };
				float s = scaleDist(rng) * 2.0f;
				obj.scale = { s, s, s };
				obj.rotation = { rotDist(rng), rotDist(rng), rotDist(rng) };
				
				XMMATRIX S = XMMatrixScaling(obj.scale.x, obj.scale.y, obj.scale.z);
				XMMATRIX R = XMMatrixRotationRollPitchYaw(obj.rotation.x, obj.rotation.y, obj.rotation.z);
				XMMATRIX T = XMMatrixTranslation(obj.position.x, obj.position.y, obj.position.z);
				obj.localWorld = S * R * T;

				m_FieldRocks[fIdx].push_back(obj);
			}
		}
	}
}

float Chunk::GetHeight(const Vector3& position)
{
	int x = (int)floorf((position.x / m_Spacing) + (m_kChunkNumX / 2.0f));
	int z = (int)floorf((position.z / m_Spacing) + (m_kChunkNumZ / 2.0f));

	if (x >= 0 && x < m_kChunkNumX && z >= 0 && z < m_kChunkNumZ)
	{
		int index = x * m_kChunkNumZ + z;
		if (index >= 0 && index < (int)m_ChildFields.size())
		{
			MeshField* field = m_ChildFields[index];
			if (field)
			{
				return field->GetHeight(position);
			}
		}
	}

	return 0.f;
}


void Chunk::UpdateTerrainObjectPositions()
{
	for (size_t fIdx = 0; fIdx < m_ChildFields.size(); fIdx++)
	{
		MeshField* field = m_ChildFields[fIdx];
		if (!field) continue;

		for (auto& tree : m_FieldTrees[fIdx])
		{
			float newHeight = field->GetHeight(tree.position);
			if (newHeight < m_HeightLimit * 2.0f)
			{
				tree.position.y = newHeight;
				XMMATRIX S = XMMatrixScaling(tree.scale.x, tree.scale.y, tree.scale.z);
				XMMATRIX R = XMMatrixRotationRollPitchYaw(tree.rotation.x, tree.rotation.y, tree.rotation.z);
				XMMATRIX T = XMMatrixTranslation(tree.position.x, tree.position.y, tree.position.z);
				tree.localWorld = S * R * T;
			}
		}

		for (auto& rock : m_FieldRocks[fIdx])
		{
			float newHeight = field->GetHeight(rock.position);
			if (newHeight < m_HeightLimit * 2.0f)
			{
				rock.position.y = newHeight;
				XMMATRIX S = XMMatrixScaling(rock.scale.x, rock.scale.y, rock.scale.z);
				XMMATRIX R = XMMatrixRotationRollPitchYaw(rock.rotation.x, rock.rotation.y, rock.rotation.z);
				XMMATRIX T = XMMatrixTranslation(rock.position.x, rock.position.y, rock.position.z);
				rock.localWorld = S * R * T;
			}
		}
	}
}

bool Chunk::IsPositionInChunk(const Vector3& position)
{
	float chunkHalfSize = (m_kChunkNumX * m_Spacing) / 2.0f;

	return (position.x >= -chunkHalfSize && position.x <= chunkHalfSize &&
		position.z >= -chunkHalfSize && position.z <= chunkHalfSize);
}


bool Chunk::IsInViewDistance(const Vector3& position)
{
	if (!m_EnableDistanceCulling) return true;

	float distance = Vector3::Distance(position, m_PlayerPosition);
	return distance <= m_ViewDistance;
}

void Chunk::UpdatePlayerPosition()
{
	auto player = Manager::GetScene()->GetGameObject<Player>();
	m_PlayerPosition = player->GetPosition();
}


Chunk::CullingResult Chunk::CheckFrustum(const Vector3& pos, float radius)
{
	if (!m_EnableFrustumCulling) return CullingResult::Inside;

	XMVECTOR p = XMVectorSet(pos.x, pos.y, pos.z, 1.0f);
	bool allInside = true;

	for (int i = 0; i < m_kFrustumPlanesNum; i++)
	{
		XMVECTOR plane = XMLoadFloat4(&m_FrustumPlanes[i]);

		XMVECTOR dot = XMPlaneDotCoord(plane, p);
		float distance = XMVectorGetX(dot);

		if (distance < -radius)
		{
			return CullingResult::Outside; 
		}
		if (distance < radius)
		{
			allInside = false; 
		}
	}

	return allInside ? CullingResult::Inside : CullingResult::Intersects;
}

void Chunk::DrawImGui()
{

	static int frameCount = -1;
	if (frameCount == ImGui::GetFrameCount()) return;
	frameCount = ImGui::GetFrameCount();

	if (!ImGui::Begin("Global Chunk Settings", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::End();
		return;
	}

	ImGui::Text("--- Culling Settings (Global) ---");
	ImGui::Checkbox("Enable Distance Culling", &m_EnableDistanceCulling);
	ImGui::Checkbox("Enable Frustum Culling", &m_EnableFrustumCulling);
	ImGui::Checkbox("Show Debug Info", &m_ShowDebugInfo);

	if (m_EnableDistanceCulling)
	{
		ImGui::SliderFloat("View Distance", &m_ViewDistance, m_kViewDistanceMin, m_kViewDistanceMax);
	}

	ImGui::Separator();
	ImGui::Text("--- Chunk Settings ---");
	ImGui::SliderFloat("Spacing", &m_Spacing, m_kSpacingMin, m_kSpacingMax);


	if (m_ShowDebugInfo)
	{
		int totalTrees = 0;
		int totalRocks = 0;
		for (const auto& vt : m_FieldTrees) totalTrees += (int)vt.size();
		for (const auto& vr : m_FieldRocks) totalRocks += (int)vr.size();

		ImGui::Separator();
		ImGui::Text("--- Render Statistics ---");
		ImGui::Text("Drawn Fields: %d / %d", m_DrawnFieldCount, (int)m_ChildFields.size());
		ImGui::Text("Drawn Trees: %d / %d", m_DrawnTreeCount, totalTrees);
		ImGui::Text("Drawn Rocks: %d / %d", m_DrawnRockCount, totalRocks);
		ImGui::Text("Draw Calls: %d", Renderer::GetDrawCallCount());
		ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
	}
	
	
	
	
	if (!m_ChildFields.empty())
	{
		m_ChildFields[m_kShowImGuiFieldNum]->DrawImGui(); 
	}

	ImGui::End();
}