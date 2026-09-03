#include "main.h"
#include "polygon.h"
#include "renderer.h"
#include "keyboard.h"



//ƒ}ƒNƒ
#define NUM_POLYGONVERTEX 4


void PolyGon::Initialize()
{
	VERTEX_3D vertex[4];

	vertex[0].Position = XMFLOAT3(0.f, 0.f, 0.f);
	vertex[0].Normal   = XMFLOAT3(0.f, 0.f, 0.f);
	vertex[0].TexCoord = XMFLOAT2(0.f, 0.f);
	vertex[0].Diffuse  = XMFLOAT4(1.f, 1.f, 1.f, 1.f);

	vertex[1].Position = XMFLOAT3(200.f, 0.f, 0.f);
	vertex[1].Normal   = XMFLOAT3(0.f, 0.f, 0.f);
	vertex[1].TexCoord = XMFLOAT2(1.f, 0.f);
	vertex[1].Diffuse  = XMFLOAT4(1.f, 1.f, 1.f, 1.f);

	vertex[2].Position = XMFLOAT3(0.f, 200.f, 0.f);
	vertex[2].Normal   = XMFLOAT3(0.f, 0.f, 0.f);
	vertex[2].TexCoord = XMFLOAT2(0.f, 1.f);
	vertex[2].Diffuse  = XMFLOAT4(1.f, 1.f, 1.f, 1.f);

	vertex[3].Position = XMFLOAT3(200.f, 200.f, 0.f);
	vertex[3].Normal   = XMFLOAT3(0.f, 0.f, 0.f);
	vertex[3].TexCoord = XMFLOAT2(1.f, 1.f);
	vertex[3].Diffuse  = XMFLOAT4(1.f, 1.f, 1.f, 1.f);

	D3D11_BUFFER_DESC bd{};
	bd.Usage          = D3D11_USAGE_DEFAULT;
	bd.ByteWidth      = sizeof(VERTEX_3D) * NUM_POLYGONVERTEX;
	bd.BindFlags      = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA sd{};
	sd.pSysMem = vertex;

	Renderer::GetDevice()->CreateBuffer(
		&bd,
		&sd,
		&m_VertexBuffer
	);

	TexMetadata metadata;
	ScratchImage image;


	LoadFromWICFile(
		L"asset\\texture\\drill.png",
		WIC_FLAGS_NONE,
		&metadata,
		image
	);


	CreateShaderResourceView(
		Renderer::GetDevice(),
		image.GetImages(),
		image.GetImageCount(),
		metadata, &m_Texture
	);
	assert(m_Texture);

	Renderer::CreateVertexShader(
		&m_VertexShader,
		&m_VertexLayout,
		"shader\\unlitTextureVS.cso"
	);


	Renderer::CreatePixelShader(
		&m_PixelShader,
		"shader\\unlitTexturePS.cso"
	);
}

void PolyGon::Update()
{
}

void PolyGon::Draw()
{
	Renderer::GetDeviceContext()->IASetInputLayout(m_VertexLayout);
	Renderer::GetDeviceContext()->VSSetShader(m_VertexShader, NULL, 0);
	Renderer::GetDeviceContext()->PSSetShader(m_PixelShader, NULL, 0);
	Renderer::SetWorldViewProjection2D();

	UINT stride = sizeof(VERTEX_3D);
	UINT offset = 0;

	Renderer::GetDeviceContext()->IASetVertexBuffers(0, 1, &m_VertexBuffer, &stride, &offset);
	Renderer::GetDeviceContext()->PSSetShaderResources(0, 1, &m_Texture);

	Renderer::GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	Renderer::AddWorldMatrix(
		{ m_Scale.x, m_Scale.y, m_Scale.z },                                    
		{ m_Position.x, m_Position.y, m_Position.z },									
		{ m_Rotation.x, m_Rotation.y, m_Rotation.z }                                  
	);

	MATERIAL material{};
	material.Diffuse = XMFLOAT4(1.f, 1.f, 1.f, 1.f);
	material.TextureEnable = true;
	Renderer::SetMaterial(material);

	Renderer::GetDeviceContext()->Draw(4, 0);
}

void PolyGon::Finalize()
{
	m_VertexBuffer->Release();

	m_PixelShader->Release();
	m_VertexShader->Release();
	m_VertexLayout->Release();

	m_Texture->Release();
}
