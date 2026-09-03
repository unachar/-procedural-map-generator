#include "main.h"
#include "manager.h"
#include "renderer.h"

#include "camera.h"
#include "player.h"
#include "gameobject.h"
#include "input.h"

#include "game.h"
#include "audio.h"
#include <list>


Scene* Manager::m_Scene = nullptr;
Scene* Manager::m_NextScene = nullptr;
bool Manager::m_IsSceneChange = false;

void Manager::Init()
{
	Renderer::Init();
	Fade::Init();

	Input::Init();
	Audio::InitMaster();

	m_Scene = new Game();
	m_Scene->Initialize();


	Fade::StartFadeIn();
}

void Manager::Uninit()
{
	
	Input::Uninit();
	Fade::Uninit();
	Renderer::Uninit();

	if (m_Scene)
	{
		m_Scene->Finalize();
		delete m_Scene;
		m_Scene = nullptr;
	}
	Audio::UninitMaster();
}

void Manager::Update()
{
	Input::Update();
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
	
	m_Scene->Update();

	Fade::Update();

	

	if (m_NextScene && Fade::IsFadeOut())
	{
		m_Scene->Finalize();
		delete m_Scene;
		m_Scene = m_NextScene;
		m_Scene->Initialize();
		m_NextScene = nullptr;
		m_IsSceneChange = false;
		Fade::StartFadeIn();
	}
	
}

void Manager::Draw()
{
	Renderer::Begin();

	m_Scene->Draw();
	Fade::Draw();
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

	Renderer::End();
}
