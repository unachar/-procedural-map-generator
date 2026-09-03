#pragma once
#include "gameobject.h"

class PolyGon : public GameObject
{
private:
	ID3D11Buffer* m_VertexBuffer;

	ID3D11PixelShader* m_PixelShader;
	ID3D11VertexShader* m_VertexShader;
	ID3D11InputLayout* m_VertexLayout;

	ID3D11ShaderResourceView* m_Texture;

	XMMATRIX m_WorldMatrix;
	XMMATRIX m_TranslationMatrix;
	XMMATRIX m_ScalingMatrix;
	XMMATRIX m_RotationMatrix;
public:
	void Initialize();
	void Update();
	void Draw();
	void Finalize();
};