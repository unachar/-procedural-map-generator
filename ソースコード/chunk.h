#pragma once  
#include "meshField.h"  
#include "vector.h"  
#include <vector>  

class Tree;  
class Rock;  

class Chunk : public GameObject  
{  

private:  
    //定数設定
	//チャンクのサイズやオブジェクト数など
   static constexpr int m_kChunkNumX = 40;
   static constexpr int m_kChunkNumZ = 40;
   static constexpr int m_kTreeNum = 50;
   static constexpr int m_kRockNum = 50;

   //フラスタム平面の数やオブジェクトのスペーシングなど
   static constexpr int m_kFrustumPlanesNum = 6;
   static constexpr float m_kSpacingOffset = 11.f;
   static constexpr float m_kRandDistNum = 50.f;

   //オブジェクトのランダムなスケールの範囲
   static constexpr float m_kRandScaleMin = 0.5f;
   static constexpr float m_kRandScaleMax = 1.5f;

   //高さ制限や表示距離の範囲
   static constexpr float m_kViewDistanceMin = 10.f;
   static constexpr float m_kViewDistanceMax = 500.f;
   static constexpr float m_kHeightScaleMin = 1.f;
   static constexpr float m_kHeightScaleMax = 50.f;

   //スペーシングの範囲
   static constexpr float m_kSpacingMin = 10.f;
   static constexpr float m_kSpacingMax = 200.f;

   // ImGuiで表示するフィールド数のオフセットや乱数シードのデフォルト値など
   static constexpr int m_kShowImGuiFieldNum = 0;
   static constexpr int m_kDefaultSeed = 0;

   //チャンクのマージンや表示距離の基本値など
   static constexpr float m_kChunkMarginBase = 10.5f;
   static constexpr float m_kViewDistanceBase = 100.f;
   static constexpr float m_kSpacingBase = 75.f;
   static constexpr float m_kHeightLimitBase = 10.f;

   //チャンクの更新に関するフレーム制限
   static constexpr int m_kMaxUpdateFrame = 5;

   //メンバ変数

   float m_Spacing;
   float m_HeightLimit;

   XMMATRIX m_WorldMatrix;  
   Vector3 m_ChunkOrigin;  

   int m_Seed;  

   float m_PosX;  
   float m_PosZ;  
   float m_PosY;  

   float m_ChunkMargin;  

   float m_ViewDistance;  
   Vector3 m_PlayerPosition;   

   // ImGui制御用フラグ  
   static bool m_EnableFrustumCulling;  
   static bool m_EnableDistanceCulling;  
   static bool m_ShowDebugInfo;  

   // 地形オブジェクト管理  
   struct TerrainObject 
   {
      Vector3 position;
      Vector3 scale;
      Vector3 rotation;
      XMMATRIX localWorld;
   };


   Tree* m_pTree;
   Rock* m_pRock;

   vector<vector<TerrainObject>> m_FieldTrees;
   vector<vector<TerrainObject>> m_FieldRocks;


   vector<Vector3> m_TreeOriginalPositions;  
   vector<Vector3> m_RockOriginalPositions;  

   XMFLOAT4 m_FrustumPlanes[m_kFrustumPlanesNum];
   int m_DrawnFieldCount;  
   int m_DrawnTreeCount;  
   int m_DrawnRockCount;  

   // パラメータ変更監視用  
   float m_PrevNoiseScale;  
   float m_PrevNoiseOctaves;  
   float m_PrevNoisePersistence;  
   float m_PrevHeightScale;  
   float m_PrevSpacing;
   float m_PrevHeightLimit;
   float m_PrevViewDistance;
   int m_PrevSeed;  

private:  
   void GenerateTerrainObjects(); // 地形オブジェクト生成  
   void UpdateTerrainObjectPositions(); // 地形変更時にオブジェクト位置を更新  
   bool IsPositionInChunk(const Vector3& position); // 位置がチャンク内かチェック  



   enum class CullingResult
   { Outside,
     Intersects,
     Inside 
   };

   CullingResult CheckFrustum(const Vector3& pos, float radius);
   bool IsInFrustum(const Vector3& pos, float radius) { return CheckFrustum(pos, radius) != CullingResult::Outside; }

public:  
   vector<MeshField*> m_ChildFields;  
   vector<MeshField*> GetChildFields() { return m_ChildFields; }  

public:  
   void Initialize() override;  
   void Finalize() override;  
   void Update() override;  
   void Draw() override;  
   void DrawImGui();  


   float GetHeight(const Vector3& position);  
   const int GetChunkNumX() const { return m_kChunkNumX; }  
   const int GetChunkNumZ() const { return m_kChunkNumZ; }  


   void SetPosition(Vector3 position) { m_Position = position; }  
   void SetRotation(Vector3 rotation) { m_Rotation = rotation; }  
   void SetScale(Vector3 scale) { m_Scale = scale; }  


   bool IsInViewDistance(const Vector3& position);  
   void UpdatePlayerPosition();  
};
