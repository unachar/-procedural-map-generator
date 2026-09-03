#pragma once

class Fade
{
public:
	static void Init();
	static void Uninit();

	static void StartFadeIn(float duration = 0.5f);
	static void StartFadeOut(float duration = 0.5f);

	static void Update();
	static void Draw();

	static bool IsActive();
	static bool IsFadeOut();
};

