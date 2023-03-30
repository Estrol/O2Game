#pragma once
#include <windows.h>

class AudioSampleChannel {
public:
	AudioSampleChannel();
	AudioSampleChannel(DWORD sampleHandle, float rate, float vol, bool pitch);

	void SetVolume(int vol);

	bool HasPlayed();
	bool Play();
	bool Stop();
	bool Pause();

	bool IsStopped();

private:
	DWORD m_hCurrentSample = NULL;

	float m_rate;
	float m_vol;

	bool m_silent;
	bool m_pitch;
	bool m_hasPlayed;
};