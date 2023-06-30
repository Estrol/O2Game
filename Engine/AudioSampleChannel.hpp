#pragma once
#include "Data/WindowsTypes.hpp"

class AudioSampleChannel {
public:
	AudioSampleChannel();
	AudioSampleChannel(DWORD sampleHandle, float rate, float vol, bool pitch);
	~AudioSampleChannel();

	void SetVolume(int vol);
	void SetPan(int pan);

	bool HasPlayed();
	bool Play();
	bool Stop();
	bool Pause();

	bool IsPlaying();
	bool IsStopped();

private:
	DWORD m_hCurrentSample = NULL;

	float m_rate;
	float m_vol;
	float m_pan;

	bool m_silent;
	bool m_pitch;
	bool m_hasPlayed;
};