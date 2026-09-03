#include "main.h"
#include "fade.h"
#include "renderer.h"
#include <cmath>
#include <algorithm>


enum class FadeState
{
	Idle,
	FadeIn,
	FadeOut,
	Full,
};

ID3D11Buffer*			g_VertexBuffer = nullptr;
ID3D11VertexShader*		g_VertexShader = nullptr;
ID3D11PixelShader*		g_PixelShader = nullptr;
ID3D11InputLayout*		g_InputLayout = nullptr;
ID3D11ShaderResourceView* g_Texture = nullptr;

FadeState	g_State = FadeState::Idle;
float		g_Timer = 0.0f;
float		g_Duration = 0.5f;
float		g_Alpha = 0.0f;
bool		g_FadeOutCompletedEvent = false;

constexpr float FADE_STEP = 1.0f / 45.0f;

void Fade::Init()
{
	VERTEX_3D vertex[4]{};

	D3D11_BUFFER_DESC bd{};
	bd.Usage = D3D11_USAGE_DYNAMIC;
	bd.ByteWidth = sizeof(vertex);
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	D3D11_SUBRESOURCE_DATA sd{};
	sd.pSysMem = vertex;

	Renderer::GetDevice()->CreateBuffer(&bd, &sd, &g_VertexBuffer);

	{
		D3D11_MAPPED_SUBRESOURCE mapped{};
		VERTEX_3D vertex[4]{};

			vertex[0].Position = XMFLOAT3(0.f, 0.f, 0.f);
			vertex[0].Normal = XMFLOAT3(0.f, 0.f, 0.f);
			vertex[0].TexCoord = XMFLOAT2(0.f, 0.f);
			vertex[0].Diffuse = XMFLOAT4(1.f, 1.f, 1.f, 1.f);

			vertex[1].Position = XMFLOAT3(SCREEN_WIDTH, 0.f, 0.f);
			vertex[1].Normal = XMFLOAT3(0.f, 0.f, 0.f);
			vertex[1].TexCoord = XMFLOAT2(1.f, 0.f);
			vertex[1].Diffuse = XMFLOAT4(1.f, 1.f, 1.f, 1.f);

			vertex[2].Position = XMFLOAT3(0.f, SCREEN_HEIGHT, 0.f);
			vertex[2].Normal = XMFLOAT3(0.f, 0.f, 0.f);
			vertex[2].TexCoord = XMFLOAT2(0.f, 1.f);
			vertex[2].Diffuse = XMFLOAT4(1.f, 1.f, 1.f, 1.f);

			vertex[3].Position = XMFLOAT3(SCREEN_WIDTH, SCREEN_HEIGHT, 0.f);
			vertex[3].Normal = XMFLOAT3(0.f, 0.f, 0.f);
			vertex[3].TexCoord = XMFLOAT2(1.f, 1.f);
			vertex[3].Diffuse = XMFLOAT4(1.f, 1.f, 1.f, 1.f);

			D3D11_BUFFER_DESC bd{};
			bd.Usage = D3D11_USAGE_DEFAULT;
			bd.ByteWidth = sizeof(VERTEX_3D) * 4;
			bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
			bd.CPUAccessFlags = 0;

			D3D11_SUBRESOURCE_DATA sd{};
			sd.pSysMem = vertex;

			Renderer::GetDevice()->CreateBuffer(
				&bd,
				&sd,
				&g_VertexBuffer
			);
	}

	Renderer::CreateVertexShader(&g_VertexShader, &g_InputLayout, "shader\\unlitTextureVS.cso");
	Renderer::CreatePixelShader(&g_PixelShader, "shader\\unlitTexturePS.cso");

	{
		TexMetadata metadata{};
		ScratchImage image{};
		auto hr = LoadFromWICFile(
			L"asset\\texture\\white.jpg",
			WIC_FLAGS_NONE,
			&metadata,
			image
		);
		if (SUCCEEDED(hr))
		{
			hr = CreateShaderResourceView(
				Renderer::GetDevice(),
				image.GetImages(),
				image.GetImageCount(),
				metadata,
				&g_Texture
			);
			assert(SUCCEEDED(hr));
		}
	}

	g_State = FadeState::Full;
	g_Timer = 0.0f;
	g_Alpha = 1.0f;
	g_FadeOutCompletedEvent = false;
}

void Fade::Uninit()
{
	if (g_VertexBuffer)
	{
		g_VertexBuffer->Release();
		g_VertexBuffer = nullptr;
	}

	if (g_VertexShader)
	{
		g_VertexShader->Release();
		g_VertexShader = nullptr;
	}

	if (g_PixelShader)
	{
		g_PixelShader->Release();
		g_PixelShader = nullptr;
	}

	if (g_InputLayout)
	{
		g_InputLayout->Release();
		g_InputLayout = nullptr;
	}

	if (g_Texture)
	{
		g_Texture->Release();
		g_Texture = nullptr;
	}

	g_State = FadeState::Idle;
	g_Alpha = 0.0f;
	g_FadeOutCompletedEvent = false;
}

void Fade::StartFadeIn(float duration)
{
	g_Duration = max(duration, 0.0001f);
	g_Timer = 0.0f;
	g_State = FadeState::FadeIn;
	g_Alpha = 1.0f;
	g_FadeOutCompletedEvent = false;
}

void Fade::StartFadeOut(float duration)
{
	g_Duration = max(duration, 0.0001f);
	g_Timer = 0.0f;
	g_State = FadeState::FadeOut;
	g_FadeOutCompletedEvent = false;
	if (duration <= 0.0f)
	{
		g_State = FadeState::Full;
		g_Alpha = 1.0f;
		g_FadeOutCompletedEvent = true;
	}
}

void Fade::Update()
{
	switch (g_State)
	{
	case FadeState::FadeIn:
		g_Timer += FADE_STEP;
		if (g_Timer >= g_Duration)
		{
			g_Timer = g_Duration;
			g_State = FadeState::Idle;
			g_Alpha = 0.0f;
		}
		else
		{
			float t = g_Timer / g_Duration;

			t = t * t * (3.0f - 2.0f * t);
			g_Alpha = 1.0f - t;
		}
		break;

	case FadeState::FadeOut:
		g_Timer += FADE_STEP;
		if (g_Timer >= g_Duration)
		{
			g_Timer = g_Duration;
			g_State = FadeState::Full;
			g_Alpha = 1.0f;
			g_FadeOutCompletedEvent = true;
		}
		else
		{
			float t = g_Timer / g_Duration;

			t = t * t * (3.0f - 2.0f * t);
			g_Alpha = t;
		}
		break;

	case FadeState::Full:
		g_Alpha = 1.0f;
		break;

	case FadeState::Idle:

	default:
		g_Alpha = 0.0f;
		break;
	}

	// アルファ値をクランプ
	g_Alpha = clamp(g_Alpha, 0.0f, 1.0f);
}

void Fade::Draw()
{
	if (!g_VertexBuffer || !g_Texture)
	{
		return;
	}

	if (g_State == FadeState::Idle && g_Alpha <= 0.0f)
	{
		return;
	}

	ID3D11DepthStencilState* prevDepthState = nullptr;
	UINT prevStencilRef = 0;
	Renderer::GetDeviceContext()->OMGetDepthStencilState(&prevDepthState, &prevStencilRef);
	Renderer::SetDepthEnable(false);

	// アルファブレンディングを有効化
	ID3D11BlendState* prevBlendState = nullptr;
	FLOAT prevBlendFactor[4];
	UINT prevSampleMask;
	Renderer::GetDeviceContext()->OMGetBlendState(&prevBlendState, prevBlendFactor, &prevSampleMask);
	
	D3D11_BLEND_DESC blendDesc{};
	blendDesc.AlphaToCoverageEnable = FALSE;
	blendDesc.IndependentBlendEnable = FALSE;
	blendDesc.RenderTarget[0].BlendEnable = TRUE;
	blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	ID3D11BlendState* blendState = nullptr;
	Renderer::GetDevice()->CreateBlendState(&blendDesc, &blendState);
	FLOAT blendFactor[4] = { 0, 0, 0, 0 };
	Renderer::GetDeviceContext()->OMSetBlendState(blendState, blendFactor, 0xFFFFFFFF);

	Renderer::GetDeviceContext()->IASetInputLayout(g_InputLayout);
	Renderer::GetDeviceContext()->VSSetShader(g_VertexShader, nullptr, 0);
	Renderer::GetDeviceContext()->PSSetShader(g_PixelShader, nullptr, 0);
	Renderer::SetWorldViewProjection2D();

	UINT stride = sizeof(VERTEX_3D);
	UINT offset = 0;
	Renderer::GetDeviceContext()->IASetVertexBuffers(0, 1, &g_VertexBuffer, &stride, &offset);
	Renderer::GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture);
	Renderer::GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	MATERIAL material{};
	material.Diffuse = XMFLOAT4(0.0f, 0.0f, 0.0f, g_Alpha);
	material.TextureEnable = TRUE;
	Renderer::SetMaterial(material);

	Renderer::GetDeviceContext()->Draw(4, 0);

	ID3D11ShaderResourceView* nullSRV = nullptr;
	Renderer::GetDeviceContext()->PSSetShaderResources(0, 1, &nullSRV);

	// ブレンド状態を復元
	Renderer::GetDeviceContext()->OMSetBlendState(prevBlendState, prevBlendFactor, prevSampleMask);
	if (blendState) blendState->Release();

	if (prevDepthState)
	{
		Renderer::GetDeviceContext()->OMSetDepthStencilState(prevDepthState, prevStencilRef);
		prevDepthState->Release();
	}
	if (prevBlendState) prevBlendState->Release();
}

bool Fade::IsActive()
{
	return g_State != FadeState::Idle || g_Alpha > 0.0f;
}

bool Fade::IsFadeOut()
{
	if (!g_FadeOutCompletedEvent)
	{
		return false;
	}

	g_FadeOutCompletedEvent = false;
	return true;
}

