#pragma once
#include <string>
#include <unordered_map>
#include "Audio.hpp"
#include "AudioSample.hpp"
#include <filesystem>

class Window;

class AudioManager {
public:
	bool Init(Window* window);

	bool Create(std::string id, uint8_t*, size_t size, Audio** out);
	bool Create(std::string id, std::filesystem::path path, Audio** out);
	bool CreateSample(std::string id, uint8_t*, size_t size, AudioSample** out);
	bool CreateSample(std::string id, std::filesystem::path path, AudioSample** out);
	bool CreateSampleFromData(std::string id, int sampleFlags, int sampleRate, int sampleChannels, int sampleLength, void* sampleData, AudioSample** out);

	void Update(double delta);

	Audio* Get(std::string id);
	AudioSample* GetSample(std::string id);

	bool Remove(std::string id);
	bool RemoveSample(std::string id);

	static AudioManager* GetInstance();
	static void Release();

private:
	AudioManager();
	~AudioManager();

	void PrepareBASS();

	static AudioManager* s_instance;
	bool m_initialized;

	double nextUpdate;

	Audio* m_bootAudio;
	AudioSample* m_bootSample;

	Window* m_currentWindow;
	std::unordered_map<std::string, Audio*> m_audios;
	std::unordered_map<std::string, AudioSample*> m_audioSamples;
};