#pragma once
#include "main.h"
#include "gameobject.h"
#include <vector>
class Room;
class CreateRoom :public GameObject
{

private:

    //定数設定
	//キューブの頂点位置やインデックス数など
    static constexpr float m_kCubePosBase = 0.5f;
	static constexpr int m_kCubeVertexCount = 8;
	static constexpr int m_kCubeIndexCount = 36;

	//cubeのカラー設定
	static constexpr XMFLOAT3 m_kNormalBase = XMFLOAT3(0.f, 0.f, 0.f); 
	static constexpr XMFLOAT2 m_kTexCoordBase = XMFLOAT2(0.f, 0.f);
	static constexpr XMFLOAT4 m_kDiffuseBase = XMFLOAT4(1.f, 1.f, 1.f, 1.f);

	

	//フラスタム平面の数や乱数の最大値など
	static constexpr int m_kPlanesNum = 6;
	static constexpr int m_kRandomMax = 100000;


	//ImGui用の定数設定
	//マップ生成の基本パラメータ
	static constexpr float m_kTileScaleBase = 1.0f;
	static constexpr float m_kWallHeightBase = 7.0f;
	static constexpr int m_kMaxStepsBase = 5000;
	static constexpr int m_kMinRoomSizeBase = 2;
	static constexpr int m_kMaxRoomSizeBase = 8;
	
	//生成パラメータの範囲設定
	static constexpr float m_kTileScaleMin = 0.5f;
	static constexpr float m_kTileScaleMax = 3.0f;
	static constexpr float m_kWallHeightMin = 3.0f;
	static constexpr float m_kWallHeightMax = 15.0f;

	//部屋生成のステップ数や部屋サイズの範囲設定
	static constexpr int m_kMaxStepsNumMin = 1000;
	static constexpr int m_kMaxStepsNumMax = 10000;
	static constexpr int m_kMinRoomSizeNumMin = 1;
	static constexpr int m_kMinRoomSizeNumMax = 5;
	static constexpr int m_kMaxRoomSizeNumMin = 5;
	static constexpr int m_kMaxRoomSizeNumMax = 15;

	//乱数シードの範囲設定
	static constexpr int m_kSeedBase = 0;
	static constexpr int m_kSeedNumMin = 0;
	static constexpr int m_kSeedNumMax = 99999;


	//レンダリング距離の範囲設定
	static constexpr float m_kMaxRenderDistanceBase = 110.0f;
	static constexpr float m_kMaxRenderDistanceMin = 50.f;
	static constexpr float m_kMaxRenderDistanceMax = 300.f;

    //メンバ変数

    Room* m_Room;

    ID3D11Buffer* m_VertexBuffer = nullptr;
    ID3D11Buffer* m_IndexBuffer = nullptr;
    ID3D11Buffer* m_InstanceBufferFloor = nullptr;
    ID3D11Buffer* m_InstanceBufferWall = nullptr;

    ID3D11VertexShader* m_VertexShader = nullptr;
    ID3D11InputLayout* m_VertexLayout = nullptr;
    ID3D11PixelShader* m_PixelShader = nullptr;

    int m_FloorInstanceCount = 0;
    int m_WallInstanceCount = 0;

    // 白塗り/黒枠用のラスタライザステート
    ID3D11RasterizerState* m_StateSolid = nullptr;
    ID3D11RasterizerState* m_StateWire = nullptr;

	XMFLOAT4 m_Planes[m_kPlanesNum]; // フラスタム平面の定数バッファ用

    // ImGui用パラメータ
    static bool s_NeedRegenerate;
    static float s_TileScale;
    static float s_WallHeight;
    static int s_MaxSteps;
    static int s_MinRoomSize;
    static int s_MaxRoomSize;
    static int s_Seed;
    static float s_MaxRenderDistance;


public:
    void Initialize()override;
    void Update()override;
    void Draw()override;
    void Finalize()override;
    void DrawImGui();

    Room* GetRoom() const { return m_Room; }

    // マップ再生成用
    void RegenerateMap();

};