#pragma once
#include <list>
#include <vector>
#include "gameobject.h"

class Scene
{
private:
	list<GameObject*> m_GameObject[3];

	bool m_pause;

public:

	virtual void Initialize();
	virtual void Update();
	virtual void Draw();
	virtual void Finalize();

	virtual void DrawImGui() {}


	template <typename T>
	 T* CreateGameObject(int layer)
	{
		T* gameObject = new T();
		gameObject->Initialize();
		m_GameObject[layer].push_back(gameObject);

		return gameObject;
	}
	template <typename T>
	 T* GetGameObject()
	{
		for (int i = 0; i < 3; ++i)
		{
			for (auto gameObject : m_GameObject[i])
			{
				T* obj = dynamic_cast<T*>(gameObject);
				if (obj)
				{
					return obj;
				}
			}
		}

		return nullptr;
	}

	template<typename T>
	 vector<T*> GetGameObjects()
	{

		vector<T*> objects;
		for (int i = 0; i < 3; i++)
		{
			for (auto gameobjects : m_GameObject[i])
			{
				T* obj = dynamic_cast<T*>(gameobjects);
				if (obj)
				{
					objects.push_back(obj);
				}
			}

		}
		return objects;

	}

	 void SetPause(bool pause) { m_pause = pause; }

	 bool GetPause() { return m_pause; }
};