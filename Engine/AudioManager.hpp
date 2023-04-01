#pragma once
#include <string>
#include <unordered_map>
#include "Audio.hpp"
#include "AudioSample.hpp"

class Window;

class AudioManager {
public:
	bool Init(Window* window);

	bool Create(std::string id, uint8_t*, size_t size, Audio** out);
	bool Create(std::string id, std::string path, Audio** out);
	bool CreateSample(std::string id, uint8_t*, size_t size, AudioSample** out);
	bool CreateSample(std::string id, std::string path, AudioSample** out);

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

	static AudioManager* s_instance;
	bool m_initialized;

	double nextUpdate;

	Window* m_currentWindow;
	std::unordered_map<std::string, Audio*> m_audios;
	std::unordered_map<std::string, AudioSample*> m_audioSamples;
};