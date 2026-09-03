#pragma once
#include <list>
#include <vector>
#include <functional>
#include "gameobject.h"
#include "fade.h"

class Manager
{
private:
	static class Scene* m_Scene;
	static class Scene* m_NextScene;
	static bool			m_IsSceneChange;
	
public:
	static void Init();
	static void Uninit();
	static void Update();
	static void Draw();

	static Scene* GetScene() { return m_Scene; }

	template<typename T>
	static void SetScene()
	{
		if (m_IsSceneChange || m_NextScene)
		{
			return;
		}

		m_NextScene = new T();
		m_IsSceneChange = true;
		Fade::StartFadeOut();
	}
	
	
};