#pragma once

struct VERTEX_3D
{
	XMFLOAT3 Position;
	XMFLOAT3 Normal;
	XMFLOAT4 Diffuse;
	XMFLOAT2 TexCoord;
};



struct MATERIAL
{
	XMFLOAT4	Ambient;
	XMFLOAT4	Diffuse;
	XMFLOAT4	Specular;
	XMFLOAT4	Emission;
	float		Shininess;
	BOOL		TextureEnable;
	float		Dummy[2];
};



struct LIGHT
{
	BOOL		Enable;
	BOOL		Dummy[3];//16byte境界
	XMFLOAT4	Direction;
	XMFLOAT4	Diffuse;
	XMFLOAT4	Ambient;

	XMFLOAT4	Position;
	XMFLOAT4	PointLightParam;

	XMFLOAT4	SkyColor;
	XMFLOAT4	GroundColor;
	XMFLOAT4	GroundNormal;

	XMFLOAT4	Angle;
	float       Length;
	float		Dummy2[3];//16byte境界
};

struct FrameBufferData
{
	BYTE* data;
	UINT width;
	UINT height;
	bool isUsed;
};


class Renderer
{
private:

	static D3D_FEATURE_LEVEL       m_FeatureLevel;
	static int                     m_DrawCallCount; // ドローコール数計測用

	static ID3D11Device*           m_Device;
	static ID3D11DeviceContext*    m_DeviceContext;
	static IDXGISwapChain*         m_SwapChain;
	static ID3D11RenderTargetView* m_RenderTargetView;
	static ID3D11DepthStencilView* m_DepthStencilView;

	static ID3D11Buffer*			m_WorldBuffer;
	static ID3D11Buffer*			m_ViewBuffer;
	static ID3D11Buffer*			m_ProjectionBuffer;
	static ID3D11Buffer*			m_MaterialBuffer;
	static ID3D11Buffer*			m_LightBuffer;


	static ID3D11DepthStencilState* m_DepthStateEnable;
	static ID3D11DepthStencilState* m_DepthStateDisable;

	static ID3D11BlendState*		m_BlendState;
	static ID3D11BlendState*		m_BlendStateATC;

public:
	static constexpr int MAX_SAVE_BUFFER = 100;
	static FrameBufferData Framebufferdata[MAX_SAVE_BUFFER];

	static void Init();
	static void Uninit();
	static void Begin();
	static void End();

	static void SetDepthEnable(bool Enable);
	static void SetATCEnable(bool Enable);
	static void SetWorldViewProjection2D();
	static void SetWorldMatrix(XMMATRIX WorldMatrix);
	static void SetViewMatrix(XMMATRIX ViewMatrix);
	static void SetProjectionMatrix(XMMATRIX ProjectionMatrix);
	static void SetMaterial(MATERIAL Material);
	static void SetLight(LIGHT Light);

	static ID3D11Device* GetDevice( void ){ return m_Device; }
	static ID3D11DeviceContext* GetDeviceContext( void ){ return m_DeviceContext; }


	static void CreateVertexShader(ID3D11VertexShader** VertexShader, ID3D11InputLayout** VertexLayout, const char* FileName);
	static void CreatePixelShader(ID3D11PixelShader** PixelShader, const char* FileName);

	static void AddWorldMatrix(XMFLOAT3 scaling = {1.f,1.f,1.f}, XMFLOAT3 translation = {}, XMFLOAT3 rotation = {});

	static void SaveBufferByPNG(const char* filename);
	static int GetEncoderClsid(const WCHAR* format, CLSID* pClsid);
	static void SaveBuffer(int index);
	static void SaveBufferToPNG(int index,int seed);

	// ドローコール計測
	static void AddDrawCall() { m_DrawCallCount++; }
	static int GetDrawCallCount() { return m_DrawCallCount; }
	static void ResetDrawCallCount() { m_DrawCallCount = 0; }
};
