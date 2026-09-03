#pragma once
class ModelRenderer;
#include "gameobject.h"
#include <vector>

struct InstanceData
{
    XMFLOAT4X4 worldMatrix;
};

class Tree : public GameObject
{
private:
    ModelRenderer* m_ModelRenderer;
    ID3D11Buffer* m_InstanceBuffer;
    vector<InstanceData> m_InstanceData;

    ID3D11VertexShader* m_VertexShader;
    ID3D11PixelShader* m_PixelShader;
    ID3D11InputLayout* m_VertexLayout;

    int m_InstanceCount = 0;
    const int MAX_INSTANCES = 200000;

public:
    void Initialize() override;
    void Update() override;
    void Draw() override;
    void Finalize() override;

    void AddInstance(const XMMATRIX& world);
    void ClearInstances();
    void UpdateInstanceBuffer();

    vector<InstanceData> GetInstanceData() { return m_InstanceData; }
};