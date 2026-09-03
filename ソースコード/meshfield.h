#pragma once
#include "renderer.h"
#include "gameObject.h"

struct VERTEX_3D;

class MeshField : public GameObject
{
private:
	//定数設定
	//マップサイズ
	static constexpr int m_kMapSizeX = 16;
	static constexpr int m_kMapSizeZ = 16;

	//ImGuiパラメーター定数設定

	static constexpr float m_kNoiseScaleBase = 1.0f;
	static constexpr float m_kNoiseOctavesBase = 9.0f;
	static constexpr float m_kNoisePersistenceBase = 0.3f;
	static constexpr float m_kHeightScaleBase = 16.0f;
	static constexpr int m_kSeedBase = 0;

	static constexpr float m_kNoiseScaleMin = 0.01f;
	static constexpr float m_kNoiseScaleMax = 0.1f;

	
	VERTEX_3D m_Vertex[m_kMapSizeX][m_kMapSizeZ];
	
	Vector3 m_WorldPosition;
	
	
	ID3D11Buffer* m_VertexBuffer = nullptr;
	ID3D11Buffer* m_IndexBuffer = nullptr;
	ID3D11InputLayout* m_VertexLayout = nullptr;
	ID3D11VertexShader* m_VertexShader = nullptr;
	ID3D11PixelShader* m_PixelShader = nullptr;
	
	ID3D11ShaderResourceView* m_Texture = nullptr;
	ID3D11ShaderResourceView* m_Texture_dirt = nullptr;

	// グリッド表示用リソース (最適化)
	ID3D11Buffer* m_GridVertexBuffer = nullptr;
	ID3D11Buffer* m_GridIndexBuffer = nullptr;
	ID3D11RasterizerState* m_GridRasterState = nullptr;
	int m_GridIndexCount = 0;
	bool m_NeedUpdateGrid = true;

public:
	// ImGuiパラメータ
	static float s_NoiseScale;
	static float s_NoiseOctaves;
	static float s_NoisePersistence;
	static float s_HeightScale;
	static int s_Seed; // 全体共有のシード
	static bool s_NeedRegenerate;
	
	static int s_NextSeed;

public:
	// インスタンスごとの状態
	int m_Seed; // このフィールドのシード
	bool m_NeedRegenerate = false; // 再生成フラグ (public化)


public:
	void Initialize() override;
	void Finalize() override;
	void Update() override;
	void Draw() override;
	
	// 高さ取得
	float GetHeight(Vector3 position) const;
	
	// 特定位置の高さを変更（掘削など）
	void SetHeightAtPosition(Vector3 position, float height);
	
	// デバッグ用グリッド描画
	void DrawGrid();
	
	// ImGui描画
	void DrawImGui();
	
	// 地形再生成（パラメータ変更時）
	void RegenerateTerrain();
	
	void SetWorldPosition(Vector3 worldPos) { m_WorldPosition = worldPos; }
	
	// シード値の設定
	void SetSeed(int seed) { m_Seed = seed; }
	int GetSeed() const { return m_Seed; }
	
	static void PrepareNextSeed(int seed) { s_NextSeed = seed; }

	ID3D11Buffer* GetVertexBuffer() const { return m_VertexBuffer; }
	ID3D11Buffer* GetIndexBuffer() const { return m_IndexBuffer; }
};
