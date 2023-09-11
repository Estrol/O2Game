#pragma once
#include "../Rendering/WindowsTypes.h"
#include <stdint.h>

class AudioSampleChannel {
#if _DEBUG
	const char SIGNATURE[25] = "AudioSampleChannel";
#endif

public:
	AudioSampleChannel();
	AudioSampleChannel(uint32_t sampleHandle, float rate, float vol, bool pitch);
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
	uint32_t m_hCurrentSample = 0;

	float m_rate;
	float m_vol;
	float m_pan;

	bool m_silent;
	bool m_pitch;
	bool m_hasPlayed;
};