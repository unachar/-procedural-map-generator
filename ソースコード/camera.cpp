#include "main.h"
#include "manager.h"
#include "Renderer.h"
#include "camera.h"
#include "player.h"
#include "input.h"
#include"scene.h"
#include "mouse.h"
#include "meshField.h"
#include "createmap.h"
#include "room.h"

void Camera::Initialize()
{
	m_Position = Vector3(0.f, 10.f, -10.f);
	m_Rotation = Vector3(0.f, 0.f, 0.f);
	m_IsActionCameraMode = false;
	m_SwitchedActionMode = false;
	m_IsScreenShot = false;
	m_ShowScreenshotDisplay = false;
	m_MouseSensitivity = 0.f;
	m_Fovy = 0.f;

    for (int i = 0; i < 10; i++)
    {
		m_NumberKeyPressed[i] = false;
    }

	

    m_MouseSensitivity = 0.005f;
    m_Fovy = 1.f;

}

void Camera::Update()
{
    if (m_IsActionCameraMode)
    {
        // マウスの相対移動量を取得
        Mouse_State state;
        Mouse_GetState(&state);
        
        if (IsMouseCursorUnlocked())
        {
            Mouse_SetMode(MOUSE_POSITION_MODE_ABSOLUTE);
        }
        else
        {
            Mouse_SetMode(MOUSE_POSITION_MODE_RELATIVE);

            if (ShouldResetMouseLook())
            {
                ClearResetMouseLook();
                state.x = 0;
                state.y = 0;
            }

            // 左右回転 (Yaw)
            m_Rotation.y += state.x * m_MouseSensitivity;

            // 上下回転 (Pitch)
            m_Rotation.x += state.y * m_MouseSensitivity;

            // 上下回転制限
            float limit = XM_PIDIV2 - 0.01f;
            if (m_Rotation.x > limit) m_Rotation.x = limit;
            if (m_Rotation.x < -limit) m_Rotation.x = -limit;
        }

        // --- プレイヤー位置を追従 ---
        Player* player = Manager::GetScene()->GetGameObject<Player>();
        m_Target = player->GetPosition() + Vector3(0.f, 1.5f, 0.f);

		Vector3 m_target2 = m_Target;

        // --- 注視点からカメラ位置を算出 ---
        float distance = 5.f;
        float x = sinf(m_Rotation.y) * cosf(m_Rotation.x) * distance;
        float y = sinf(m_Rotation.x) * distance;
        float z = cosf(m_Rotation.y) * cosf(m_Rotation.x) * distance;

		Vector3 desiredPos = m_Target + Vector3(-x, y, -z);


        static Vector3 startPos;
        static float lerpTime = 0.0f;
        if (m_SwitchedActionMode)
        {
            startPos = m_Position;
            lerpTime = 0.0f;
            m_SwitchedActionMode = false;
        }


        float speed = 3.0f;
        lerpTime += speed * (1.0f / 60.0f);
		if (lerpTime < 1.0f)
		{
			m_Position = m_Position.Lerp(startPos, desiredPos, lerpTime);
		}
		else
		{
			m_Position = player->GetPosition() + Vector3(-x, y + 1.5f, -z);
		}

		
    }
    ZoomInOut();
	TakeScreenShot();
}

void Camera::Draw()
{
	m_Projection = XMMatrixPerspectiveFovLH(
		m_Fovy ,
		(float)SCREEN_WIDTH / SCREEN_HEIGHT,
		1.f,
		10000.f
	);

	Renderer::SetProjectionMatrix(m_Projection);

	XMFLOAT3 up = XMFLOAT3(0.f, 1.f, 0.f);
	m_View = XMMatrixLookAtLH(
		XMVectorSet(m_Position.x,m_Position.y,m_Position.z,1.f),
		XMVectorSet(m_Target.x,m_Target.y,m_Target.z,1.f),
		XMLoadFloat3(&up)
	);

	Renderer::SetViewMatrix(m_View);
}

void Camera::Finalize()
{
}

void Camera::ZoomInOut()
{

    if (Input::GetKeyPress(VK_UP)) 
    {
		m_Fovy -= 0.03f;
		if (m_Fovy < 0.1f) m_Fovy = 0.1f;
    }
    if (Input::GetKeyPress(VK_DOWN)) 
    {
        m_Fovy += 0.03f;
        if (m_Fovy > 3.0f) m_Fovy = 3.0f;
    }
}

void Camera::TakeScreenShot()
{

	if (Input::GetKeyTrigger('M'))
	{

		auto mesh = Manager::GetScene()->GetGameObject<MeshField>();
		int currentSeed = mesh->GetSeed();


		for (int i = 0; i < 100; i++)
		{
			if (!Renderer::Framebufferdata[i].isUsed)
			{

				Renderer::SaveBuffer(i);

				Renderer::SaveBufferToPNG(i, currentSeed);
				break;
			}
		}
	}


	for (int i = 0; i <= 9; i++) 
	{
		if (Input::GetKeyTrigger('0' + i)) 
		{

			if (Renderer::Framebufferdata[i].isUsed)
			{
				auto mesh = Manager::GetScene()->GetGameObject<MeshField>();
				int currentSeed = mesh ? mesh->GetSeed() : 0;

				Renderer::SaveBufferToPNG(i, currentSeed);
			}
		}
	}
}

void Camera::DrawScreenshotDisplay()
{

	if (Input::GetKeyTrigger('V'))
	{
		m_ShowScreenshotDisplay = !m_ShowScreenshotDisplay;
	}

	if (m_ShowScreenshotDisplay)
	{
		ImGui::Begin("Screenshot Gallery##Camera", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
		
		ImGui::Text("Press V to toggle this window");
		ImGui::Text("Press M to take screenshots (0-9 to save as PNG)");
		ImGui::Separator();
		

		int columns = 4;
		float buttonSize = 120.0f;
		int screenshotCount = 0;
		
		for (int i = 0; i < 100; i++)
		{
			if (Renderer::Framebufferdata[i].isUsed)
			{
				if (screenshotCount % columns == 0)
				{
					if (screenshotCount > 0)
						ImGui::NewLine();
				}
				

				string buttonLabel = "Shot " + to_string(i);
				
				if (ImGui::Button(buttonLabel.c_str(), ImVec2(buttonSize, buttonSize)))
				{

					auto mesh = Manager::GetScene()->GetGameObject<MeshField>();
					int currentSeed = mesh ? mesh->GetSeed() : 0;

					Renderer::SaveBufferToPNG(i, currentSeed);
				}
				
				ImGui::SameLine();
				screenshotCount++;
			}
		}
		
		if (screenshotCount == 0)
		{
			ImGui::Text("No screenshots taken yet.");
			ImGui::Text("Press M to take a screenshot!");
		}
		else
		{
			ImGui::NewLine();
			ImGui::Text("Total screenshots: %d/100", screenshotCount);
		}
		
		ImGui::End();
	}
}

