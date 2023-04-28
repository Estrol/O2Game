#pragma once
#include "AudioSampleChannel.hpp"
#include <iostream>
#include <filesystem>

class AudioSample {
public:
	AudioSample(std::string id);
	~AudioSample();

	bool Create(uint8_t* buffer, size_t size);
	bool Create(std::filesystem::path path);
	bool CreateFromData(int sampleFlags, int sampleRate, int sampleChannels, int sampleLength, void* sampleData);
	bool CreateSilent();
	void SetRate(double rate);

	std::string GetId() const;

	std::unique_ptr<AudioSampleChannel> CreateChannel();

private:
	void CheckAudioTime();
	std::string m_id;
	
	DWORD m_handle;
	
	float m_rate;
	float m_vol;

	bool m_silent;
	bool m_pitch;
};