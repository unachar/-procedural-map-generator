#pragma once

#include "scene.h"

class Game : public Scene
{
private:
	class Audio* m_Bgm = nullptr;
public:
	void Initialize() override;
	void Finalize() override;
	void Update() override;
	void Draw() override;
};