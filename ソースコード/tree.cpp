#include "tree.h"
#include "renderer.h"
#include "modelRenderer.h"

LIGHT Light;

void Tree::Initialize()
{
	m_ModelRenderer = new ModelRenderer;
	m_ModelRenderer->Load("asset\\model\\tree\\tree.obj");

	Renderer::CreateVertexShader(
		&m_VertexShader,
		&m_VertexLayout,
		"shader\\treeInstanceVS.cso"
	);


	Renderer::CreatePixelShader(
		&m_PixelShader,
		"shader\\vertextoonPS.cso"
	);

	Light.Diffuse = XMFLOAT4(2.f, 2.f, 2.f, 1.0f);
	Light.Ambient = XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);
	Light.Direction = XMFLOAT4(0.2f, -1.f, 0.0f, 0.0f);

	Light.Position = XMFLOAT4(0.0f, 1.5f, 1.0f, 1.0f);
	Light.PointLightParam = XMFLOAT4(5000.0f, 1.5f, 0.0f, 0.0f);

	// インスタンスバッファ作成
	D3D11_BUFFER_DESC bd = {};
	bd.Usage = D3D11_USAGE_DYNAMIC;
	bd.ByteWidth = sizeof(InstanceData) * MAX_INSTANCES;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	Renderer::GetDevice()->CreateBuffer(&bd, nullptr, &m_InstanceBuffer);
}

void Tree::Update()
{
}

void Tree::Draw()
{
	if (m_InstanceCount == 0) return;

	Renderer::SetLight(Light);
	Renderer::GetDeviceContext()->IASetInputLayout(m_VertexLayout);
	Renderer::GetDeviceContext()->VSSetShader(m_VertexShader, NULL, 0);
	Renderer::GetDeviceContext()->PSSetShader(m_PixelShader, NULL, 0);



	// 頂点バッファ + インスタンスバッファ設定
	UINT strides[2] = { sizeof(VERTEX_3D), sizeof(InstanceData) };
	UINT offsets[2] = { 0, 0 };

	ID3D11Buffer* vbs[2] = { m_ModelRenderer->GetVertexBuffer(), m_InstanceBuffer };
	Renderer::GetDeviceContext()->IASetVertexBuffers(0, 2, vbs, strides, offsets);


	Renderer::GetDeviceContext()->IASetIndexBuffer(m_ModelRenderer->GetIndexBuffer(), DXGI_FORMAT_R32_UINT, 0);
	Renderer::GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);


	MODEL* mdl = m_ModelRenderer->GetModel();
	for (unsigned int i = 0; i < mdl->SubsetNum; i++)
	{
		Renderer::SetMaterial(mdl->SubsetArray[i].Material.Material);
		if (mdl->SubsetArray[i].Material.Texture)
			Renderer::GetDeviceContext()->PSSetShaderResources(0, 1, &mdl->SubsetArray[i].Material.Texture);


		Renderer::GetDeviceContext()->DrawIndexedInstanced(
			mdl->SubsetArray[i].IndexNum,
			m_InstanceCount,
			mdl->SubsetArray[i].StartIndex,
			0, 0);


	}
	Renderer::AddDrawCall();
}

void Tree::Finalize()
{
	if (m_InstanceBuffer) m_InstanceBuffer->Release();
	m_VertexLayout->Release();
	m_VertexShader->Release();
	m_PixelShader->Release();
	m_ModelRenderer->Uninit();
}

void Tree::ClearInstances()
{
	m_InstanceData.clear();
	m_InstanceCount = 0;
}

void Tree::AddInstance(const XMMATRIX& world)
{
	if (m_InstanceCount >= MAX_INSTANCES) return;

	InstanceData data;
	XMStoreFloat4x4(&data.worldMatrix, XMMatrixTranspose(world));
	m_InstanceData.push_back(data);
	m_InstanceCount++;
}

void Tree::UpdateInstanceBuffer()
{
	if (m_InstanceCount == 0) return;

	D3D11_MAPPED_SUBRESOURCE msr;
	Renderer::GetDeviceContext()->Map(m_InstanceBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);
	memcpy(msr.pData, m_InstanceData.data(), sizeof(InstanceData) * m_InstanceCount);
	Renderer::GetDeviceContext()->Unmap(m_InstanceBuffer, 0);
}
