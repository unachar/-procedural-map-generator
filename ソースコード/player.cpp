#include "main.h"
#include "renderer.h"

#include "player.h"

#include "input.h"
#include "camera.h"
#include "manager.h"

#include"scene.h"
#include "mouse.h"
#include <algorithm>


void Player::Initialize()
{
	m_Position = Vector3(0.f, 0.f, -5.f);
	m_Rotation = Vector3(0.f, 0.f, 0.f);
	m_Scale = { 0.01f, 0.01f, 0.01f };

	m_MoveSpeed = 0.3f;
	m_DashSpeed = 1.5f;

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

void Player::Update()
{

	if (Manager::GetScene()->GetPause())
	{
		return;
	}


	Camera* camera = Manager::GetScene()->GetGameObject<Camera>();
	Vector3 rotation = camera->GetRotation();

	PlayerAction(camera, rotation);
}

void Player::Draw()
{
}


void Player::Finalize()
{

	m_VertexLayout->Release();
	m_VertexShader->Release();
	m_PixelShader->Release();
}

void Player::PlayerAction(Camera* camera, Vector3 rotation)
{
	Vector3 moveDirection = Vector3(0.f, 0.f, 0.f);


	if (Input::GetKeyPress('W')) moveDirection += camera->GetForward() * 1.f;
	if (Input::GetKeyPress('S')) moveDirection += camera->GetForward() * -1.f;
	if (Input::GetKeyPress('A')) moveDirection += camera->GetRight() * -1.f;
	if (Input::GetKeyPress('D')) moveDirection += camera->GetRight() * 1.f;


	if (Input::GetKeyPress('E')) moveDirection.y += 1.f; 
	if (Input::GetKeyPress('Q')) moveDirection.y -= 1.f; 

	float currentSpeed = Input::GetKeyPress(VK_LSHIFT) ? m_DashSpeed : m_MoveSpeed;

	if (moveDirection.Length() > 0)
	{
		moveDirection.Normalize();
		Vector3 velocity = moveDirection * currentSpeed;

		m_Position += velocity;
	}
}