#pragma once
#include <vector>
#include<random>
#include "main.h"

//map設定 (デフォルト値)
static constexpr int GRID_SIZE = 1000;
static constexpr float TILE_SCALE = 1.f;
static constexpr float WALL_HEIGHT = 7.f;
static constexpr int MAX_STEPS = 5000;

// マップ生成の固定スタートポイント（グリッド座標）
// 必ずこのマスから通路・部屋の生成を始める
static constexpr int START_GRID_X = GRID_SIZE / 2;
static constexpr int START_GRID_Y = GRID_SIZE / 2;

enum TileType
{
	EMPTY = 0,
	FLOOR = 1
};

struct InstanceData
{
	XMFLOAT4X4 worldMatrix;
};

class Room
{
private:
	vector<vector<int>> m_Grid;
	mt19937 m_Rng;


	int m_StartX = START_GRID_X;
	int m_StartY = START_GRID_Y;

	float m_MinWorldX = 0.0f;
	float m_MaxWorldX = 0.0f;
	float m_MinWorldZ = 0.0f;
	float m_MaxWorldZ = 0.0f;


	int m_GridSize;
	float m_TileScale;
	float m_WallHeight;
	int m_MaxSteps;
	int m_MinRoomSize;
	int m_MaxRoomSize;
	int m_MoveDist;
	int m_Seed;

public:
	Room() : m_Grid(GRID_SIZE, vector<int>(GRID_SIZE, EMPTY))
	{
		random_device rd;
		m_Rng.seed(rd());

		// デフォルト値で初期化
		m_GridSize = GRID_SIZE;
		m_TileScale = TILE_SCALE;
		m_WallHeight = WALL_HEIGHT;
		m_MaxSteps = MAX_STEPS;
		m_MinRoomSize = 2;
		m_MaxRoomSize = 8;
		m_MoveDist = 3;
		m_Seed = rd();
	}


	void SetGridSize(int size)
	{
		m_GridSize = size;

		m_Grid.resize(m_GridSize, vector<int>(m_GridSize, EMPTY));
	}
	void SetTileScale(float scale) { m_TileScale = scale; }
	void SetWallHeight(float height) { m_WallHeight = height; }
	void SetMaxSteps(int steps) { m_MaxSteps = steps; }
	void SetMinRoomSize(int size) { m_MinRoomSize = size; }
	void SetMaxRoomSize(int size) { m_MaxRoomSize = size; }
	void SetMoveDist(int dist) { m_MoveDist = dist; }
	void SetSeed(int seed)
	{
		m_Seed = seed;
		m_Rng.seed(seed);
	}


	int GetGridSize() const { return m_GridSize; }
	float GetTileScale() const { return m_TileScale; }
	float GetWallHeight() const { return m_WallHeight; }
	int GetMaxSteps() const { return m_MaxSteps; }
	int GetMinRoomSize() const { return m_MinRoomSize; }
	int GetMaxRoomSize() const { return m_MaxRoomSize; }
	int GetMoveDist() const { return m_MoveDist; }
	int GetSeed() const { return m_Seed; }

	void CarveRoom(int centerX, int centerY, int width, int height)
	{
		int startX = max(1, centerX - width / 2);
		int startY = max(1, centerY - height / 2);
		int endX = min(m_GridSize - 2, centerX + width / 2);
		int endY = min(m_GridSize - 2, centerY + height / 2);

		for (int y = startY; y <= endY; y++)
		{
			for (int x = startX; x <= endX; x++)
			{
				m_Grid[y][x] = FLOOR;
			}
		}
	}

	void Generate()
	{
		for (auto& row : m_Grid)
		{

			fill(row.begin(), row.end(), EMPTY);
		}

		m_StartX = clamp(START_GRID_X, 2, m_GridSize - 3);
		m_StartY = clamp(START_GRID_Y, 2, m_GridSize - 3);

		int currentX = m_StartX;
		int currentY = m_StartY;


		CarveRoom(currentX, currentY, 3, 3);


		uniform_int_distribution<int> dirDist(0, 4);
		uniform_int_distribution<int> roomSizeDist(m_MinRoomSize, m_MaxRoomSize);

		for (int i = 0; i < m_MaxSteps; ++i)
		{
			int w = roomSizeDist(m_Rng);
			int h = roomSizeDist(m_Rng);
			CarveRoom(currentX, currentY, w, h);

			int dir = dirDist(m_Rng);

			switch (dir)
			{
			case 0:
				currentY -= m_MoveDist;
				break;
			case 1:
				currentY += m_MoveDist;
				break;
			case 2:
				currentX -= m_MoveDist;
				break;
			case 3:
				currentX += m_MoveDist;
				break;
			}

			currentX = clamp(currentX, 2, m_GridSize - 3);
			currentY = clamp(currentY, 2, m_GridSize - 3);
		}


		bool first = true;
		for (int y = 1; y < m_GridSize - 1; ++y)
		{
			for (int x = 1; x < m_GridSize - 1; ++x)
			{
				if (m_Grid[y][x] == FLOOR)
				{
					float fx = (x - m_GridSize * 0.5f) * m_TileScale;
					float fz = (y - m_GridSize * 0.5f) * m_TileScale;
					if (first)
					{
						m_MinWorldX = m_MaxWorldX = fx;
						m_MinWorldZ = m_MaxWorldZ = fz;
						first = false;
					}
					else
					{
						m_MinWorldX = min(m_MinWorldX, fx);
						m_MaxWorldX = max(m_MaxWorldX, fx);
						m_MinWorldZ = min(m_MinWorldZ, fz);
						m_MaxWorldZ = max(m_MaxWorldZ, fz);
					}
				}
			}
		}
	}

	XMFLOAT3 GetStartWorldPosition() const
	{
		float fx = (m_StartX - m_GridSize * 0.5f) * m_TileScale;
		float fz = (m_StartY - m_GridSize * 0.5f) * m_TileScale;

		return XMFLOAT3(fx, 0.0f, fz);
	}

	// マップのワールド境界（床の広がり）を取得
	float GetMinWorldX() const { return m_MinWorldX; }
	float GetMaxWorldX() const { return m_MaxWorldX; }
	float GetMinWorldZ() const { return m_MinWorldZ; }
	float GetMaxWorldZ() const { return m_MaxWorldZ; }


	bool IsWalkable(const XMFLOAT3& worldPos, float radius = 0.0f) const
	{

		if (radius <= 0.0f)
		{
			int gx = static_cast<int>(worldPos.x / m_TileScale + m_GridSize * 0.5f);
			int gy = static_cast<int>(worldPos.z / m_TileScale + m_GridSize * 0.5f);
			if (gx < 1 || gx >= m_GridSize - 1 || gy < 1 || gy >= m_GridSize - 1)
			{
				return false;
			}
			return (m_Grid[gy][gx] == FLOOR);
		}


		const float offsets[4][2] = {
			{ -radius, -radius },
			{  radius, -radius },
			{ -radius,  radius },
			{  radius,  radius }
		};
		for (int i = 0; i < 4; ++i)
		{
			float px = worldPos.x + offsets[i][0];
			float pz = worldPos.z + offsets[i][1];
			int gx = static_cast<int>(px / m_TileScale + m_GridSize * 0.5f);
			int gy = static_cast<int>(pz / m_TileScale + m_GridSize * 0.5f);
			if (gx < 1 || gx >= m_GridSize - 1 || gy < 1 || gy >= m_GridSize - 1)
			{
				return false;
			}
			if (m_Grid[gy][gx] != FLOOR)
			{
				return false;
			}
		}
		return true;
	}

	void CreateRnderData(
		vector<InstanceData>& outFloors,
		vector<InstanceData>& outWalls,
		const XMFLOAT4* frustumPlanes = nullptr,
		const XMFLOAT3* cameraPos = nullptr,
		float maxDistance = 0.0f)
	{
		outFloors.clear();
		outWalls.clear();

		for (int y = 1; y < m_GridSize - 1; ++y)
		{
			for (int x = 1; x < m_GridSize - 1; ++x)
			{
				if (m_Grid[y][x] == FLOOR)
				{
					float fx = (x - m_GridSize * 0.5f) * m_TileScale;
					float fy = (y - m_GridSize * 0.5f) * m_TileScale;

					if (frustumPlanes)
					{

						XMFLOAT3 center(fx, m_WallHeight * 0.5f, fy);

						float halfTile = m_TileScale * 0.5f;
						float radius = sqrtf(halfTile * halfTile + (m_WallHeight * 0.5f) * (m_WallHeight * 0.5f) + halfTile * halfTile);
						bool inside = true;

						for (int pi = 0; pi < 6; ++pi)
						{
							const XMFLOAT4& p = frustumPlanes[pi];
							float dist = p.x * center.x + p.y * center.y + p.z * center.z + p.w;
							if (dist < -radius)
							{
								inside = false;
								break;
							}
						}
						if (!inside)
						{
							continue;
						}
					}

					if (cameraPos && maxDistance > 0.0f)
					{
						float dx = fx - cameraPos->x;
						float dy = 0.0f; // 高さ方向の差は無視する場合
						float dz = fy - cameraPos->z;
						float distSq = dx * dx + dy * dy + dz * dz;
						float maxDistSq = maxDistance * maxDistance;
						if (distSq > maxDistSq)
						{
							continue;
						}
					}

					XMFLOAT4X4 floorWorld;
					XMMATRIX floorScale = XMMatrixScaling(m_TileScale, 1.f, m_TileScale);
					XMStoreFloat4x4(&floorWorld, XMMatrixTranspose(floorScale * XMMatrixTranslation(fx, 0, fy)));
					outFloors.push_back({ floorWorld });

					XMFLOAT4X4 ceilingWorld;
					XMMATRIX ceilingScale = XMMatrixScaling(m_TileScale, 1.f, m_TileScale);
					XMStoreFloat4x4(&ceilingWorld, XMMatrixTranspose(ceilingScale * XMMatrixTranslation(fx, m_WallHeight, fy)));
					outFloors.push_back({ ceilingWorld });


					int dx[] = { 0,0,-1,1 };
					int dy[] = { -1,1,0,0 };

					float angles[] = { 0.f,XM_PI,XM_PIDIV2,-XM_PIDIV2 };

					for (int i = 0; i < 4; ++i)
					{
						if (m_Grid[y + dy[i]][x + dx[i]] == EMPTY)
						{
							XMMATRIX scale = XMMatrixScaling(1.f, m_WallHeight, 1.f);
							XMMATRIX rotation = XMMatrixRotationY(angles[i]);

							XMMATRIX offset = XMMatrixTranslation(dx[i] * m_TileScale * 0.5f, m_WallHeight * 0.5f, dy[i] * m_TileScale * 0.5f);
							XMMATRIX translation = XMMatrixTranslation(fx, 0, fy);

							XMFLOAT4X4 wallWorld;
							XMStoreFloat4x4(&wallWorld, XMMatrixTranspose(scale * rotation * offset * translation));
							outWalls.push_back({ wallWorld });
						}
					}
				}
			}
		}
	}
};