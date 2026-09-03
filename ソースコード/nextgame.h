#pragma once

#include "scene.h"

class NextGame : public Scene
{
private:
	class Audio* m_Bgm = nullptr;
	class Room* m_Room = nullptr;
public:
	void Initialize() override;
	void Finalize()override;
	void Update() override;
};