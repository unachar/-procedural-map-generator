#include "manager.h"

#include "game.h"
#include "camera.h"
#include "player.h"

#include "input.h"
#include "mouse.h"

#include "chunk.h"
#include "nextgame.h"
#include <random>

void Game::Initialize()
{
    CreateGameObject<Camera>(0);
    CreateGameObject<Chunk>(1);
    CreateGameObject<Player>(1);

    auto player = GetGameObject<Player>();
    if (player)
    {
        player->SetPosition({ 0.f, 0.f, -5.f }); 
    }
    

    auto camera = GetGameObject<Camera>();
    if (camera)
    {
        camera->SetActionMode(true);
    }
   
}


void Game::Finalize()
{
	Scene::Finalize();
}

void Game::Update()
{
	Scene::Update();
	if (Input::GetKeyTrigger(VK_RETURN))
	{
		Manager::SetScene<NextGame>();
	}
}

void Game::Draw()
{
	Scene::Draw();

    auto chunk = Manager::GetScene()->GetGameObject<Chunk>();
    if (chunk)
    {
        chunk->DrawImGui();
    }
}