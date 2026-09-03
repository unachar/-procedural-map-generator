#pragma once

#include <xaudio2.h>
#include "gameObject.h"



class Audio : public GameObject
{
private:
	static IXAudio2*				m_Xaudio;
	static IXAudio2MasteringVoice*	m_MasteringVoice;

	IXAudio2SourceVoice*	m_SourceVoice{};
	BYTE*					m_SoundData{};

	int						m_Length{};
	int						m_PlayLength{};




public:
	static void InitMaster();
	static void UninitMaster();

	using GameObject::GameObject;

	void Uninit();

	void Load(const char *FileName);
	void Play(bool Loop = false);
	void Stop();

};

