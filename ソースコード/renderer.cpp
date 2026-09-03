#define _HAS_STD_BYTE 0
#include "main.h"
#include <io.h>
#include "renderer.h"
#include <string>
#include <windows.h>
#include <gdiplus.h>
#include <ShlObj.h>

#pragma comment (lib, "gdiplus.lib")
#pragma warning(disable : 4996)

using namespace Gdiplus;

D3D_FEATURE_LEVEL       Renderer::m_FeatureLevel = D3D_FEATURE_LEVEL_11_0;

ID3D11Device*           Renderer::m_Device{};
ID3D11DeviceContext*    Renderer::m_DeviceContext{};
IDXGISwapChain*         Renderer::m_SwapChain{};
ID3D11RenderTargetView* Renderer::m_RenderTargetView{};
ID3D11DepthStencilView* Renderer::m_DepthStencilView{};

ID3D11Buffer*			Renderer::m_WorldBuffer{};
ID3D11Buffer*			Renderer::m_ViewBuffer{};
ID3D11Buffer*			Renderer::m_ProjectionBuffer{};
ID3D11Buffer*			Renderer::m_MaterialBuffer{};
ID3D11Buffer*			Renderer::m_LightBuffer{};


ID3D11DepthStencilState* Renderer::m_DepthStateEnable{};
ID3D11DepthStencilState* Renderer::m_DepthStateDisable{};


ID3D11BlendState*		Renderer::m_BlendState{};
ID3D11BlendState*		Renderer::m_BlendStateATC{};

ULONG_PTR m_GdiPlusToken;

int                     Renderer::m_DrawCallCount = 0;
FrameBufferData Renderer::Framebufferdata[MAX_SAVE_BUFFER]{};

void Renderer::Init()
{
	HRESULT hr = S_OK;

	m_GdiPlusToken = 0;


	// GDI+初期化
	GdiplusStartupInput gdiplusStartupInput;
	Status status = GdiplusStartup(&m_GdiPlusToken, &gdiplusStartupInput, NULL);
	if (status != Ok)
		return;


	// デバイス、スワップチェーン作成
	DXGI_SWAP_CHAIN_DESC swapChainDesc{};
	swapChainDesc.BufferCount = 1;
	swapChainDesc.BufferDesc.Width = SCREEN_WIDTH;
	swapChainDesc.BufferDesc.Height = SCREEN_HEIGHT;
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
	swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.OutputWindow = GetWindow();
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;
	swapChainDesc.Windowed = TRUE;

	hr = D3D11CreateDeviceAndSwapChain( NULL,
										D3D_DRIVER_TYPE_HARDWARE,
										NULL,
										0,
										NULL,
										0,
										D3D11_SDK_VERSION,
										&swapChainDesc,
										&m_SwapChain,
										&m_Device,
										&m_FeatureLevel,
										&m_DeviceContext );






	// レンダーターゲットビュー作成
	ID3D11Texture2D* renderTarget{};
	m_SwapChain->GetBuffer( 0, __uuidof( ID3D11Texture2D ), ( LPVOID* )&renderTarget );
	m_Device->CreateRenderTargetView( renderTarget, NULL, &m_RenderTargetView );
	renderTarget->Release();


	// デプスステンシルバッファ作成
	ID3D11Texture2D* depthStencile{};
	D3D11_TEXTURE2D_DESC textureDesc{};
	textureDesc.Width = swapChainDesc.BufferDesc.Width;
	textureDesc.Height = swapChainDesc.BufferDesc.Height;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_D16_UNORM;
	textureDesc.SampleDesc = swapChainDesc.SampleDesc;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = 0;
	m_Device->CreateTexture2D(&textureDesc, NULL, &depthStencile);

	// デプスステンシルビュー作成
	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc{};
	depthStencilViewDesc.Format = textureDesc.Format;
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	depthStencilViewDesc.Flags = 0;
	m_Device->CreateDepthStencilView(depthStencile, &depthStencilViewDesc, &m_DepthStencilView);
	depthStencile->Release();


	m_DeviceContext->OMSetRenderTargets(1, &m_RenderTargetView, m_DepthStencilView);



	// ビューポート設定
	D3D11_VIEWPORT viewport{};
	viewport.Width = (FLOAT)SCREEN_WIDTH;
	viewport.Height = (FLOAT)SCREEN_HEIGHT;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	m_DeviceContext->RSSetViewports( 1, &viewport );



	// ラスタライザステート設定
	D3D11_RASTERIZER_DESC rasterizerDesc{};
	rasterizerDesc.FillMode = D3D11_FILL_SOLID; 
	rasterizerDesc.CullMode = D3D11_CULL_BACK; 
	rasterizerDesc.DepthClipEnable = TRUE; 
	rasterizerDesc.MultisampleEnable = FALSE; 

	ID3D11RasterizerState *rs;
	m_Device->CreateRasterizerState( &rasterizerDesc, &rs );

	m_DeviceContext->RSSetState( rs );




	// ブレンドステート設定
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

	m_Device->CreateBlendState( &blendDesc, &m_BlendState );

	blendDesc.AlphaToCoverageEnable = TRUE;
	m_Device->CreateBlendState( &blendDesc, &m_BlendStateATC );

	float blendFactor[4] = {0.0f, 0.0f, 0.0f, 0.0f};
	m_DeviceContext->OMSetBlendState(m_BlendState, blendFactor, 0xffffffff );





	// デプスステンシルステート設定
	D3D11_DEPTH_STENCIL_DESC depthStencilDesc{};
	depthStencilDesc.DepthEnable = TRUE;
	depthStencilDesc.DepthWriteMask	= D3D11_DEPTH_WRITE_MASK_ALL;
	depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
	depthStencilDesc.StencilEnable = FALSE;

	m_Device->CreateDepthStencilState( &depthStencilDesc, &m_DepthStateEnable );//深度有効ステート

	depthStencilDesc.DepthEnable = FALSE;
	depthStencilDesc.DepthWriteMask	= D3D11_DEPTH_WRITE_MASK_ZERO;
	m_Device->CreateDepthStencilState( &depthStencilDesc, &m_DepthStateDisable );//深度無効ステート

	m_DeviceContext->OMSetDepthStencilState( m_DepthStateEnable, NULL );




	// サンプラーステート設定
	D3D11_SAMPLER_DESC samplerDesc{};
	samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.MaxAnisotropy = 4;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	ID3D11SamplerState* samplerState{};
	m_Device->CreateSamplerState( &samplerDesc, &samplerState );

	m_DeviceContext->PSSetSamplers( 0, 1, &samplerState );



	// 定数バッファ生成
	D3D11_BUFFER_DESC bufferDesc{};
	bufferDesc.ByteWidth = sizeof(XMFLOAT4X4);
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bufferDesc.CPUAccessFlags = 0;
	bufferDesc.MiscFlags = 0;
	bufferDesc.StructureByteStride = sizeof(float);

	m_Device->CreateBuffer( &bufferDesc, NULL, &m_WorldBuffer );
	m_DeviceContext->VSSetConstantBuffers( 0, 1, &m_WorldBuffer);

	m_Device->CreateBuffer( &bufferDesc, NULL, &m_ViewBuffer );
	m_DeviceContext->VSSetConstantBuffers( 1, 1, &m_ViewBuffer );

	m_Device->CreateBuffer( &bufferDesc, NULL, &m_ProjectionBuffer );
	m_DeviceContext->VSSetConstantBuffers( 2, 1, &m_ProjectionBuffer );


	bufferDesc.ByteWidth = sizeof(MATERIAL);

	m_Device->CreateBuffer( &bufferDesc, NULL, &m_MaterialBuffer );
	m_DeviceContext->VSSetConstantBuffers( 3, 1, &m_MaterialBuffer );
	m_DeviceContext->PSSetConstantBuffers( 3, 1, &m_MaterialBuffer );


	bufferDesc.ByteWidth = sizeof(LIGHT);

	m_Device->CreateBuffer( &bufferDesc, NULL, &m_LightBuffer );
	m_DeviceContext->VSSetConstantBuffers( 4, 1, &m_LightBuffer );
	m_DeviceContext->PSSetConstantBuffers( 4, 1, &m_LightBuffer );





	// ライト初期化
	LIGHT light{};
	light.Enable = true;
	light.Direction = XMFLOAT4(0.0f, -1.0f, 0.5f, 0.0f);
	light.Ambient = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	light.Diffuse = XMFLOAT4(1.5f, 1.5f, 1.5f, 1.0f);
	SetLight(light);



	// マテリアル初期化
	MATERIAL material{};
	material.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	material.Ambient = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	SetMaterial(material);




}



void Renderer::Uninit()
{

	m_WorldBuffer->Release();
	m_ViewBuffer->Release();
	m_ProjectionBuffer->Release();
	m_LightBuffer->Release();
	m_MaterialBuffer->Release();


	m_DeviceContext->ClearState();
	m_RenderTargetView->Release();
	m_SwapChain->Release();
	m_DeviceContext->Release();
	m_Device->Release();
	// GDI+終了
	if (m_GdiPlusToken != 0)
	{
		GdiplusShutdown(m_GdiPlusToken);
		m_GdiPlusToken = 0;
	}
}




void Renderer::Begin()
{
	ResetDrawCallCount();
	float clearColor[4] = { 0.8f, 0.8f, 0.2f, 1.0f };
	m_DeviceContext->ClearRenderTargetView( m_RenderTargetView, clearColor );
	m_DeviceContext->ClearDepthStencilView( m_DepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);
}



void Renderer::End()
{
	// VSyncを有効化（SyncInterval=1）してちらつき・ティアリングを防止
	m_SwapChain->Present( 1, 0 );
}




void Renderer::SetDepthEnable( bool Enable )
{
	if( Enable )
		m_DeviceContext->OMSetDepthStencilState( m_DepthStateEnable, NULL );
	else
		m_DeviceContext->OMSetDepthStencilState( m_DepthStateDisable, NULL );

}



void Renderer::SetATCEnable( bool Enable )
{
	float blendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

	if (Enable)
		m_DeviceContext->OMSetBlendState(m_BlendStateATC, blendFactor, 0xffffffff);
	else
		m_DeviceContext->OMSetBlendState(m_BlendState, blendFactor, 0xffffffff);

}

void Renderer::SetWorldViewProjection2D()
{
	SetWorldMatrix(XMMatrixIdentity());
	SetViewMatrix(XMMatrixIdentity());

	XMMATRIX projection;
	projection = XMMatrixOrthographicOffCenterLH(0.0f, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f, 0.0f, 1.0f);
	SetProjectionMatrix(projection);
}


void Renderer::SetWorldMatrix(XMMATRIX WorldMatrix)
{
	XMFLOAT4X4 worldf;
	XMStoreFloat4x4(&worldf, XMMatrixTranspose(WorldMatrix));
	m_DeviceContext->UpdateSubresource(m_WorldBuffer, 0, NULL, &worldf, 0, 0);
}

void Renderer::SetViewMatrix(XMMATRIX ViewMatrix)
{
	XMFLOAT4X4 viewf;
	XMStoreFloat4x4(&viewf, XMMatrixTranspose(ViewMatrix));
	m_DeviceContext->UpdateSubresource(m_ViewBuffer, 0, NULL, &viewf, 0, 0);
}

void Renderer::SetProjectionMatrix(XMMATRIX ProjectionMatrix)
{
	XMFLOAT4X4 projectionf;
	XMStoreFloat4x4(&projectionf, XMMatrixTranspose(ProjectionMatrix));
	m_DeviceContext->UpdateSubresource(m_ProjectionBuffer, 0, NULL, &projectionf, 0, 0);

}



void Renderer::SetMaterial( MATERIAL Material )
{
	m_DeviceContext->UpdateSubresource( m_MaterialBuffer, 0, NULL, &Material, 0, 0 );
}

void Renderer::SetLight( LIGHT Light )
{
	m_DeviceContext->UpdateSubresource(m_LightBuffer, 0, NULL, &Light, 0, 0);
}





void Renderer::CreateVertexShader( ID3D11VertexShader** VertexShader, ID3D11InputLayout** VertexLayout, const char* FileName )
{

	FILE* file;
	long int fsize;

	file = fopen(FileName, "rb");
	assert(file);

	fsize = _filelength(_fileno(file));
	unsigned char* buffer = new unsigned char[fsize];
	fread(buffer, fsize, 1, file);
	fclose(file);

	m_Device->CreateVertexShader(buffer, fsize, NULL, VertexShader);


	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 4 * 3, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 4 * 6, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 4 * 10, D3D11_INPUT_PER_VERTEX_DATA, 0 },

		{"WORLD",0,DXGI_FORMAT_R32G32B32A32_FLOAT,1,0,D3D11_INPUT_PER_INSTANCE_DATA,1 },
		{"WORLD",1,DXGI_FORMAT_R32G32B32A32_FLOAT,1,16,D3D11_INPUT_PER_INSTANCE_DATA,1 },
		{"WORLD",2,DXGI_FORMAT_R32G32B32A32_FLOAT,1,32,D3D11_INPUT_PER_INSTANCE_DATA,1 },
		{"WORLD",3,DXGI_FORMAT_R32G32B32A32_FLOAT,1,48,D3D11_INPUT_PER_INSTANCE_DATA,1 }


	};
	UINT numElements = ARRAYSIZE(layout);

	m_Device->CreateInputLayout(layout,
		numElements,
		buffer,
		fsize,
		VertexLayout);

	delete[] buffer;
}



void Renderer::CreatePixelShader( ID3D11PixelShader** PixelShader, const char* FileName )
{
	FILE* file;
	long int fsize;

	file = fopen(FileName, "rb");
	assert(file);

	fsize = _filelength(_fileno(file));
	unsigned char* buffer = new unsigned char[fsize];
	fread(buffer, fsize, 1, file);
	fclose(file);

	m_Device->CreatePixelShader(buffer, fsize, NULL, PixelShader);

	delete[] buffer;
}

void Renderer::AddWorldMatrix(XMFLOAT3 scaling, XMFLOAT3 rotation, XMFLOAT3 translation)
{
	XMMATRIX World;
	XMMATRIX Scaling;
	XMMATRIX Translation;
	XMMATRIX Rotation;

	Scaling = XMMatrixScaling(scaling.x, scaling.y, scaling.z);
	
	Rotation = XMMatrixRotationRollPitchYaw(rotation.x,  rotation.y, rotation.z);

	Translation = XMMatrixTranslation(translation.x, translation.y, translation.z);
	

	World = Scaling * Rotation * Translation;

	SetWorldMatrix(World);
}

void Renderer::SaveBufferByPNG(const char* filename)
{
	//バックバッファからテクスチャを取得
	ID3D11Texture2D* backBufferTexture = nullptr;
	HRESULT hr = m_SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&backBufferTexture);
	if (FAILED(hr))
	{
		return;
	}

	//テクスチャの情報を取得
	D3D11_TEXTURE2D_DESC desc;
	backBufferTexture->GetDesc(&desc);

	//CPU読み取り可能なテクスチャを作成
	D3D11_TEXTURE2D_DESC cpuDesc = desc;
	cpuDesc.Usage = D3D11_USAGE_STAGING;
	cpuDesc.BindFlags = 0;
	cpuDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
	cpuDesc.MiscFlags = 0;

	ID3D11Texture2D* cpuTexture = nullptr;
	hr = m_Device->CreateTexture2D(&cpuDesc, nullptr, &cpuTexture);
	if (FAILED(hr))
	{
		backBufferTexture->Release();
		return;
	}

	//GPUからCPUへテクスチャをコピー
	m_DeviceContext->CopyResource(cpuTexture, backBufferTexture);

	//テクスチャの内容をマップ
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	hr = m_DeviceContext->Map(cpuTexture, 0, D3D11_MAP_READ, 0, &mappedResource);
	if (FAILED(hr))
	{
		cpuTexture->Release();
		backBufferTexture->Release();
		return;
	}

	//ファイル名をワイド文字列に変換
	int wide = MultiByteToWideChar(CP_ACP, 0, filename, -1, nullptr, 0);
	wstring wideFilename(wide, L'\0');
	MultiByteToWideChar(CP_ACP, 0, filename, -1, &wideFilename[0], wide);

	//ピクセルデータを変換
	BYTE* ConvertedData = new BYTE[desc.Width * desc.Height * 4];
	BYTE* SourceData = (BYTE*)mappedResource.pData;
	for (UINT y = 0; y < desc.Height; y++)
	{
		for (UINT x = 0; x < desc.Width; x++)
		{
			UINT sourceIndex = y * mappedResource.RowPitch + x * 4;
			UINT destIndex = y * desc.Width * 4 + x * 4;

			// BGRAからRGBAに変換
			ConvertedData[destIndex + 0] = SourceData[sourceIndex + 2]; // R
			ConvertedData[destIndex + 1] = SourceData[sourceIndex + 1]; // G
			ConvertedData[destIndex + 2] = SourceData[sourceIndex + 0]; // B
			ConvertedData[destIndex + 3] = 255;                         // A
		}
	}

	//ビットマップ作成
	Bitmap* bitmap = new Bitmap(desc.Width, desc.Height, desc.Width * 4,
		(PixelFormat)0x26200A, ConvertedData);

	// PNGとして保存
	CLSID pngClsid;
	if (GetEncoderClsid(L"image/png", &pngClsid) >= 0)
	{
		bitmap->Save(wideFilename.c_str(), &pngClsid, NULL);
	}

	// リソースを解放
	delete bitmap;
	delete[] ConvertedData;
	m_DeviceContext->Unmap(cpuTexture, 0);
	cpuTexture->Release();
	backBufferTexture->Release();

}

int Renderer::GetEncoderClsid(const WCHAR* format, CLSID* pClsid)
{
	UINT num = 0;
	UINT size = 0;

	GetImageEncodersSize(&num, &size);
	if (size == 0)
	{
		return -1;
	}
		

	ImageCodecInfo* pImageCodecInfo = new ImageCodecInfo[size];
	if (pImageCodecInfo == NULL)
	{
		return -2;
	}
		

	GetImageEncoders(num, size, pImageCodecInfo);

	for (UINT j = 0; j < num; ++j)
	{
		if (wcscmp(pImageCodecInfo[j].MimeType, format) == 0)
		{
			*pClsid = pImageCodecInfo[j].Clsid;
			delete[] pImageCodecInfo;
			return j;
		}
	}

	delete[] pImageCodecInfo;
	return -3;
}

void Renderer::SaveBuffer(int index)
{
	if (index < 0 || index >= MAX_SAVE_BUFFER)
	{
		return;
	}

	ID3D11Texture2D* backBufferTexture = nullptr;
	HRESULT hr = m_SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&backBufferTexture);
	if (FAILED(hr))
	{
		return;
	}

	//テクスチャの情報を取得
	D3D11_TEXTURE2D_DESC desc;
	backBufferTexture->GetDesc(&desc);

	//CPU読み取り可能なテクスチャを作成
	D3D11_TEXTURE2D_DESC cpuDesc = desc;
	cpuDesc.Usage = D3D11_USAGE_STAGING;
	cpuDesc.BindFlags = 0;
	cpuDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
	cpuDesc.MiscFlags = 0;

	ID3D11Texture2D* cpuTexture = nullptr;
	hr = m_Device->CreateTexture2D(&cpuDesc, nullptr, &cpuTexture);
	if (FAILED(hr))
	{
		backBufferTexture->Release();
		return;
	}

	//GPUからCPUへテクスチャをコピー
	m_DeviceContext->CopyResource(cpuTexture, backBufferTexture);

	//テクスチャの内容をマップ
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	hr = m_DeviceContext->Map(cpuTexture, 0, D3D11_MAP_READ, 0, &mappedResource);
	if (FAILED(hr))
	{
		cpuTexture->Release();
		backBufferTexture->Release();
		return;
	}

	//既存のデータを保存
	if (Framebufferdata[index].isUsed)
	{
		delete[] Framebufferdata[index].data;
	}
	//新しいデータを保存
	Framebufferdata[index].width = desc.Width;
	Framebufferdata[index].height = desc.Height;
	Framebufferdata[index].data = new BYTE[desc.Width * desc.Height * 4];
	Framebufferdata[index].isUsed = true;

	//ピクセルデータを変換
	BYTE* SourceData = (BYTE*)mappedResource.pData;
	for (UINT y = 0; y < desc.Height; y++)
	{
		for (UINT x = 0; x < desc.Width; x++)
		{
			UINT sourceIndex = y * mappedResource.RowPitch + x * 4;
			UINT destIndex = y * desc.Width * 4 + x * 4;

			// BGRAからRGBAに変換
			Framebufferdata[index].data[destIndex + 0] = SourceData[sourceIndex + 2]; // R
			Framebufferdata[index].data[destIndex + 1] = SourceData[sourceIndex + 1]; // G
			Framebufferdata[index].data[destIndex + 2] = SourceData[sourceIndex + 0]; // B
			Framebufferdata[index].data[destIndex + 3] = 255;                         // A
		}
	}

	// リソースを解放
	m_DeviceContext->Unmap(cpuTexture, 0);
	cpuTexture->Release();
	backBufferTexture->Release();
}

void Renderer::SaveBufferToPNG(int index,int seed)
{
	if (index < 0 || index >= MAX_SAVE_BUFFER || !Framebufferdata[index].isUsed)
		return;

	// 実行ファイルのパスを取得
	wchar_t exePath[MAX_PATH];
	GetModuleFileNameW(NULL, exePath, MAX_PATH);

	// 実行ファイルのディレクトリパスを取得
	wchar_t currentDir[MAX_PATH];
	wcscpy_s(currentDir, MAX_PATH, exePath);
	wchar_t* lastSlash = wcsrchr(currentDir, L'\\');
	if (lastSlash)
	{
		*lastSlash = L'\0';
	}

	// assetフォルダを探す（最大5階層まで上に遡る）
	wchar_t assetPath[MAX_PATH];
	wchar_t searchPath[MAX_PATH];
	bool found = false;
	
	for (int i = 0; i < 5; i++)
	{
		swprintf_s(searchPath, MAX_PATH, L"%s\\asset", currentDir);
		DWORD dwAttrib = GetFileAttributesW(searchPath);
		if (dwAttrib != INVALID_FILE_ATTRIBUTES && (dwAttrib & FILE_ATTRIBUTE_DIRECTORY))
		{
			wcscpy_s(assetPath, MAX_PATH, searchPath);
			found = true;
			break;
		}
		
		// 1階層上に移動
		lastSlash = wcsrchr(currentDir, L'\\');
		if (!lastSlash)
			break;
		*lastSlash = L'\0';
	}

	// assetフォルダが見つからない場合は実行ファイルのディレクトリから2階層上を試す
	if (!found)
	{
		GetModuleFileNameW(NULL, exePath, MAX_PATH);
		wcscpy_s(currentDir, MAX_PATH, exePath);
		lastSlash = wcsrchr(currentDir, L'\\');
		if (lastSlash) *lastSlash = L'\0';
		lastSlash = wcsrchr(currentDir, L'\\');
		if (lastSlash) *lastSlash = L'\0';
		swprintf_s(assetPath, MAX_PATH, L"%s\\asset", currentDir);
	}

	// asset/ScreenShotsフォルダのパスを構築
	wchar_t screenshotsPath[MAX_PATH];
	swprintf_s(screenshotsPath, MAX_PATH, L"%s\\ScreenShots", assetPath);

	// フォルダが存在しない場合は作成
	DWORD dwAttrib = GetFileAttributesW(screenshotsPath);
	if (dwAttrib == INVALID_FILE_ATTRIBUTES || !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY))
	{
		CreateDirectoryW(screenshotsPath, NULL);
	}

	// ファイル名を生成（screenshot_001.png形式）
	wchar_t filename[MAX_PATH];
	swprintf_s(filename, MAX_PATH, L"%s\\screenshot_%03d.png", screenshotsPath, seed);

	// Framebufferdataのデータを使ってPNGファイルを保存
	FrameBufferData& bufferData = Framebufferdata[index];
	
	// ビットマップ作成
	Bitmap* bitmap = new Bitmap(
		bufferData.width, 
		bufferData.height, 
		bufferData.width * 4,
		(PixelFormat)0x26200A,
		bufferData.data
	);

	// PNGとして保存
	CLSID pngClsid;
	if (GetEncoderClsid(L"image/png", &pngClsid) >= 0)
	{
		bitmap->Save(filename, &pngClsid, NULL);
	}

	// リソースを解放
	delete bitmap;
}



