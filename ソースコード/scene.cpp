#include "scene.h"
#include "main.h"
#include "manager.h"
#include "renderer.h"
#include "camera.h"

void Scene::Initialize()
{
	m_pause = false;
}

void Scene::Finalize()
{
	

	for (int i = 0; i < 3; i++)
	{
		for (auto GameObject : m_GameObject[i])
		{
			GameObject->Finalize();
			delete GameObject;
		}
		m_GameObject[i].clear();
	}

}

void Scene::Update()
{
	
	for (int i = 0; i < 3; i++)
	{
		for (auto Gameobject : m_GameObject[i])
		{
			if (Gameobject)
			{
				Gameobject->Update();
			}
		}

		for (int i = 0; i < 3; i++)
		{
			m_GameObject[i].remove_if([](GameObject* obj)
				{ return obj->Destroy(); });
		}

	}

}

void Scene::Draw()
{
	

	Camera* camera = GetGameObject<Camera>();
	if (camera != nullptr)
	{
		Vector3 cameraposition = camera->GetPosition();
		m_GameObject[1].sort([&](GameObject* object1, GameObject* object2)
			{
				return object1->GetDistance(cameraposition) > object2->GetDistance(cameraposition);
			});
	}
	

	for (int i = 0; i < 3; i++)
	{
		for (auto GameObject : m_GameObject[i])
		{
			if (GameObject)
			{
				GameObject->Draw();
			}
		}
	}

}


