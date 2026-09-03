#include "nextgame.h"
#include "input.h"
#include "manager.h"

#include "scene.h"

#include "camera.h"
#include "createmap.h"
#include "player.h"
#include "game.h"


void NextGame::Initialize()
{
	CreateGameObject<Camera>(0);
	CreateGameObject<CreateRoom>(1);
	CreateGameObject<Player>(1);


	auto camera = Manager::GetScene()->GetGameObject<Camera>();
	auto roomobj = Manager::GetScene()->GetGameObject<CreateRoom>();
	auto player = Manager::GetScene()->GetGameObject<Player>();


	if (camera)
	{
		camera->SetActionMode(true);
	}
}

void NextGame::Finalize()
{
}

void NextGame::Update()
{
	Scene::Update();
	if (Input::GetKeyTrigger(VK_RETURN))
	{
		Manager::SetScene<Game>();
	}
}
