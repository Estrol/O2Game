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

	std::string GetId() const;

	std::unique_ptr<AudioSampleChannel> CreateChannel();

private:
	std::string m_id;

	uint8_t* m_buffer;
	size_t m_size;
	
	DWORD m_handle;
	
	float m_rate;
	float m_vol;

	bool m_silent;
	bool m_pitch;
};