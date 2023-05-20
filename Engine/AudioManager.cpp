#include "AudioManager.hpp"
#include "Window.hpp"
#include <bass.h>
#include <bass_fx.h>
#include <sstream>
#include <fstream>
#include <vector>
#include "AudioSample.hpp"
#include "Data/bass_ogg_silent.hpp"

#define DEFAULT_SAMPLE_RATE 44100

AudioManager::AudioManager() {
	m_initialized = false;
	m_currentWindow = nullptr;
	m_audios = std::unordered_map<std::string, Audio*>();
	m_audioSamples = std::unordered_map<std::string, AudioSample*>();
}

AudioManager::~AudioManager() {
	for (auto& it : m_audios) {
		if (it.second->GetType() == AudioType::SAMPLE) {
			delete (AudioSample*)it.second;
		}
		else {
			delete it.second;
		}
	}
	
	if (m_initialized) {
		BASS_Free();
		m_initialized = false;
	}
}

AudioManager* AudioManager::s_instance = nullptr;

bool AudioManager::Init(Window* window) {
	m_currentWindow = window;

	if (HIWORD(BASS_GetVersion()) != BASSVERSION) {
		MessageBoxA(0, "An incorrect version of BASS.DLL was loaded (2.4 is required)", "Incorrect BASS.DLL", MB_ICONERROR);
		return false;
	}

	if (HIWORD(BASS_FX_GetVersion()) != BASSVERSION) {
		MessageBoxA(0, "An incorrect version of BASS_FX.DLL was loaded (2.4 is required)", "Incorrect BASS_FX.DLL", MB_ICONERROR);
		return false;
	}

	if (!BASS_Init(-1, DEFAULT_SAMPLE_RATE, BASS_DEVICE_DEFAULT, m_currentWindow->GetHandle(), NULL)) {
		MessageBoxA(0, "Can't initialize device", 0, MB_ICONERROR);
		return false;
	}

	//BASS_SetConfig(BASS_CONFIG_UPDATEPERIOD, 0);
	BASS_SetConfig(BASS_CONFIG_DEV_DEFAULT, TRUE);
	BASS_SetConfig(BASS_CONFIG_DEV_NONSTOP, TRUE);

	::printf("BASS and BASS_FX initialized\n");

	// Prepare the BASS thread to play without delay.
	PrepareBASS();
	m_initialized = true;
	return true;
}

void AudioManager::PrepareBASS() {
	Create("BOOT", BASS_AUDIO_BOOT_DATA, BASS_AUDIO_BOOT_SIZE, &m_bootAudio);
	m_bootAudio->SetVolume(0);
	m_bootAudio->Play();
	m_bootAudio->Pause();

	CreateSample("BOOT_SAMPLE", BASS_AUDIO_BOOT_DATA, BASS_AUDIO_BOOT_SIZE, &m_bootSample);
	auto t = m_bootSample->CreateChannel();
	t->SetVolume(0);
	t->Play();
}

bool AudioManager::Create(std::string id, uint8_t* buffer, size_t size, Audio** out) {
	if (size == 0) return false;

	if (m_audios.find(id) != m_audios.end()) {
		return false;
	}

	Audio* audio = new Audio(id);
	if (!audio->Create((uint8_t*)buffer, size)) {
		delete audio;
		return false;
	}

	m_audios[id] = audio;
	*out = audio;
	
	return true;
}

bool AudioManager::Create(std::string id, std::filesystem::path path, Audio** out) {
	if (m_audios.find(id) != m_audios.end()) {
		return false;
	}

	Audio* audio = new Audio(id);
	if (!path.empty()) {
		if (!audio->Create(path)) {
			delete audio;
			return false;
		}
	}

	m_audios[id] = audio;
	*out = audio;

	return true;
}

bool AudioManager::CreateSample(std::string id, uint8_t* buffer, size_t size, AudioSample** out) {
	if (size == 0) return false;

	if (m_audioSamples.find(id) != m_audioSamples.end()) {
		return false;
	}

	AudioSample* audio = new AudioSample(id);
	if (!audio->Create((uint8_t*)buffer, size)) {
		delete audio;
		return false;
	}

	m_audioSamples[id] = audio;
	*out = m_audioSamples[id];

	return true;
}

bool AudioManager::CreateSample(std::string id, std::filesystem::path path, AudioSample** out) {
	if (m_audioSamples.find(id) != m_audioSamples.end()) {
		delete m_audioSamples[id];
	}

	AudioSample* audio = new AudioSample(id);
	if (path.empty()) {
		if (!audio->CreateSilent()) {
			delete audio;
			return false;
		}
	}
	else {
		if (!audio->Create(path)) {
			int errCode = BASS_ErrorGetCode();

			delete audio;
			return false;
		}
	}

	m_audioSamples[id] = audio;
	*out = audio;

	return true;
}

bool AudioManager::CreateSampleFromData(std::string id, int sampleFlags, int sampleRate, int sampleChannels, int sampleLength, void* sampleData, AudioSample** out) {
	if (m_audioSamples.find(id) != m_audioSamples.end()) {
		delete m_audioSamples[id];
	}

	AudioSample* audio = new AudioSample(id);
	if (sampleLength == 0) {
		if (!audio->CreateSilent()) {
			delete audio;
			return false;
		}
	}
	else {
		if (!audio->CreateFromData(sampleFlags, sampleRate, sampleChannels, sampleLength, sampleData)) {
			delete audio;
			return false;
		}
	}

	m_audioSamples[id] = audio;
	*out = audio;

	return true;
}

void AudioManager::Update(double delta) {
	nextUpdate += delta;

	if (nextUpdate >= 0.2) {
		nextUpdate = 0;
		BASS_Update(1000);
	}
}

Audio* AudioManager::Get(std::string id) {
	if (m_audios.find(id) == m_audios.end()) {
		return nullptr;
	}

	return m_audios[id];
}

AudioSample* AudioManager::GetSample(std::string id) {
	if (m_audioSamples.find(id) == m_audioSamples.end()) {
		return nullptr;
	}

	return m_audioSamples[id];
}

bool AudioManager::Remove(std::string id) {
	if (m_audios.find(id) == m_audios.end()) {
		return false;
	}

	Audio* audio = m_audios[id];
	if (audio != nullptr) {
		delete audio;
		m_audios.erase(id);
	}

	return true;
}

bool AudioManager::RemoveSample(std::string id) {
	if (m_audioSamples.find(id) == m_audioSamples.end()) {
		return false;
	}

	AudioSample* sample = m_audioSamples[id];
	if (sample != nullptr) {
		delete sample;
		m_audioSamples.erase(id);
	}

	return true;
}

bool AudioManager::RemoveAll() {
	for (auto& sample : m_audioSamples) {
		delete sample.second;
	}

	m_audioSamples.clear();
	return true;
}

AudioManager* AudioManager::GetInstance() {
	if (s_instance == nullptr) {
		s_instance = new AudioManager;
	}

	return s_instance;
}

void AudioManager::Release() {
	if (s_instance != nullptr) {
		delete s_instance;
		s_instance = nullptr;
	}
}
