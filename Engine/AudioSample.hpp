#pragma once
#include "AudioSampleChannel.hpp"
#include <iostream>

class AudioSample {
public:
	AudioSample(std::string id);
	~AudioSample();

	bool Create(uint8_t* buffer, size_t size);
	bool Create(std::string path);
	bool CreateSilent();
	void SetRate(double rate);

	std::string GetId() const;

	std::unique_ptr<AudioSampleChannel> CreateChannel();

private:
	std::string m_id;
	
	DWORD m_handle;
	
	float m_rate;
	float m_vol;

	bool m_silent;
	bool m_pitch;
};