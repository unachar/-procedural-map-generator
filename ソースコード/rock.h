#pragma once
class ModelRenderer;
#include "gameobject.h"
#include <vector>

struct RockInstanceData
{
	XMFLOAT4X4 worldMatrix;
};

class Rock : public GameObject
{
private:
	ModelRenderer* m_ModelRenderer;
	ID3D11Buffer* m_InstanceBuffer;
	vector<RockInstanceData> m_InstanceData;

	ID3D11VertexShader* m_VertexShader;
	ID3D11PixelShader* m_PixelShader;
	ID3D11InputLayout* m_VertexLayout;

	int m_InstanceCount = 0;
	const int MAX_INSTANCES = 200000;

public:
	void Initialize();
	void Update();
	void Draw();
	void Finalize();

	void AddInstance(const XMMATRIX& world);
	void ClearInstances();
	void UpdateInstanceBuffer();

    ModelRenderer* GetModelRenderer() { return m_ModelRenderer; }
};