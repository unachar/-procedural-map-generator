#pragma once



// マテリアル構造体
struct MODEL_MATERIAL
{
	char						Name[256];
	MATERIAL					Material;
	char						TextureName[256];
	ID3D11ShaderResourceView*	Texture;

};


// 描画サブセット構造体
struct SUBSET
{
	unsigned int	StartIndex;
	unsigned int	IndexNum;
	MODEL_MATERIAL	Material;
};


// モデル構造体
struct MODEL_OBJ
{
	VERTEX_3D		*VertexArray;
	unsigned int	VertexNum;

	unsigned int	*IndexArray;
	unsigned int	IndexNum;

	SUBSET			*SubsetArray;
	unsigned int	SubsetNum;
};

struct MODEL
{
	ID3D11Buffer*	VertexBuffer;
	ID3D11Buffer*	IndexBuffer;

	SUBSET*			SubsetArray;
	unsigned int	SubsetNum;
};


#include "component .h"
#include <string>
#include <unordered_map>


class ModelRenderer : public Component
{
private:

	static unordered_map<string, MODEL*> m_ModelPool;

	static void LoadModel(const char *FileName, MODEL *Model);
	static void LoadObj( const char *FileName, MODEL_OBJ *ModelObj );
	static void LoadMaterial( const char *FileName, MODEL_MATERIAL **MaterialArray, unsigned int *MaterialNum );

	MODEL* m_Model{};

public:

	static void Preload( const char *FileName );
	static void UnloadAll();


	using Component::Component;

	void Load( const char *FileName );
	void Draw() override;
	MODEL* GetModel() { return m_Model; }

	ID3D11Buffer* GetVertexBuffer() { return m_Model->VertexBuffer; }
	ID3D11Buffer* GetIndexBuffer() { return m_Model->IndexBuffer; }

	// 最初のサブセットの情報を取得（多くの木モデルは単一サブセットです）
	unsigned int GetIndexCount() { return m_Model->SubsetArray[0].IndexNum; }
	unsigned int GetStartIndex() { return m_Model->SubsetArray[0].StartIndex; }

	ID3D11ShaderResourceView* GetTexture(int subsetIndex = 0) {
		return m_Model->SubsetArray[subsetIndex].Material.Texture;
	}

	MATERIAL GetMaterial(int subsetIndex = 0) {
		return m_Model->SubsetArray[subsetIndex].Material.Material;
	}

};