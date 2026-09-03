#pragma once
#include "gameobject.h"
#include <vector>
#include <string>

// ImGui用のテクスチャ構造体
struct ScreenshotTexture
{
	ID3D11ShaderResourceView* textureView = nullptr;
	int width = 0;
	int height = 0;
	bool isValid = false;
};

class Camera : public GameObject
{
private:
	float m_MinYPosition;  // カメラの最小Y座標
	float m_PlayerHeightOffset;  // プレイヤーからの高さオフセット
	float m_TargetHeightOffset;  // 注視点の高さオフセット
	float m_MinDistanceFromPlayer;  // プレイヤーからの最小距離
	XMMATRIX m_Projection;
	XMMATRIX m_View;

	Vector3 m_Target{ 0.f,0.f,0.f };
	Vector3 m_Offset{ 0.f,5.f,-10.f };
	XMFLOAT3 m_Up{ 0.f,1.f,0.f };

	bool m_IsActionCameraMode;
	bool m_SwitchedActionMode;
	bool m_IsScreenShot;
	bool m_NumberKeyPressed[10];
	bool m_ShowScreenshotDisplay;

	float m_MouseSensitivity;
	float m_Fovy;

	// スクリーンショットテクスチャ管理
	vector<ScreenshotTexture> m_ScreenshotTextures;

public:
	void Initialize();
	void Update();
	void Draw();
	void Finalize();

	void SetActionMode(bool use) 
	{
		if (use && !m_IsActionCameraMode)
		{
			m_SwitchedActionMode = true;
		}
		m_IsActionCameraMode = use;
		
	}

	void ZoomInOut();
	void TakeScreenShot();
	void DrawScreenshotDisplay();

	void SetTarget(Vector3 target) { m_Target = target; }

	Vector3 GetPosition() const { return m_Position; }
	Vector3 GetTarget() const { return m_Target; }
	XMFLOAT3 GetUpVector() const { return m_Up; }
	float GetFovy() const { return m_Fovy; }

	bool GetActionMode() { return m_IsActionCameraMode; }

	XMMATRIX GetViewMatrix() { return m_View; }
};