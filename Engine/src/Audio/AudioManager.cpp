#include "Audio/AudioManager.h"
#include "Rendering/Window.h"
#include <bass.h>
#include <bass_fx.h>
#include <sstream>
#include <fstream>
#include <vector>
#include "MsgBox.h"
#include "Audio/AudioSample.h"
#include "Misc/bass_ogg_silent.hpp"

constexpr auto DEFAULT_SAMPLE_RATE = 48000;

AudioManager::AudioManager() {
	m_initialized = false;
	m_currentWindow = nullptr;
	m_audios = std::unordered_map<std::string, std::unique_ptr<Audio>>();
	m_audioSamples = std::unordered_map<std::string, std::unique_ptr<AudioSample>>();
}

AudioManager::~AudioManager() {
	m_audios.clear();
	m_audioSamples.clear();
	
	if (m_initialized) {
		BASS_Free();
		m_initialized = false;
	}
}

AudioManager* AudioManager::s_instance = nullptr;

bool AudioManager::Init(GameWindow* window) {
	m_currentWindow = window;

	if (HIWORD(BASS_GetVersion()) != BASSVERSION) {
		MsgBox::ShowOut("An incorrect version of BASS was loaded (2.4 is required)", "Incorrect BASS", MsgBoxType::OK, MsgBoxFlags::BTN_ERROR);
		return false;
	}

	if (HIWORD(BASS_FX_GetVersion()) != BASSVERSION) {
		MsgBox::ShowOut("An incorrect version of BASS was loaded (2.4 is required)", "Incorrect BASS", MsgBoxType::OK, MsgBoxFlags::BTN_ERROR);
		return false;
	}

	if (!BASS_Init(-1, DEFAULT_SAMPLE_RATE, BASS_DEVICE_STEREO, NULL, NULL)) {
		MsgBox::ShowOut("Failed to init BASS", "Incorrect BASS", MsgBoxType::OK, MsgBoxFlags::BTN_ERROR);
		return false;
	}

	//BASS_SetConfig(BASS_CONFIG_UPDATEPERIOD, 0);
	BASS_SetConfig(BASS_CONFIG_DEV_DEFAULT, TRUE);
	BASS_SetConfig(BASS_CONFIG_DEV_NONSTOP, TRUE);
	BASS_SetConfig(BASS_CONFIG_FLOATDSP, TRUE);

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

	std::unique_ptr<Audio> audio = std::make_unique<Audio>(id);
	if (!audio->Create((uint8_t*)buffer, size)) {
		return false;
	}

	m_audios[id] = std::move(audio);
	*out = m_audios[id].get();
	
	return true;
}

bool AudioManager::Create(std::string id, std::filesystem::path path, Audio** out) {
	if (m_audios.find(id) != m_audios.end()) {
		return false;
	}

	std::unique_ptr<Audio> audio = std::make_unique<Audio>(id);
	if (!path.empty()) {
		if (!audio->Create(path)) {
			return false;
		}
	}

	m_audios[id] = std::move(audio);
	*out = m_audios[id].get();

	return true;
}

bool AudioManager::CreateSample(std::string id, uint8_t* buffer, size_t size, AudioSample** out) {
	if (size == 0) return false;

	if (m_audioSamples.find(id) != m_audioSamples.end()) {
		return false;
	}

	std::unique_ptr<AudioSample> audio = std::make_unique<AudioSample>(id);
	if (!audio->Create((uint8_t*)buffer, size)) {
		return false;
	}

	m_audioSamples[id] = std::move(audio);
	*out = m_audioSamples[id].get();

	return true;
}

bool AudioManager::CreateSample(std::string id, std::filesystem::path path, AudioSample** out) {
	if (m_audioSamples.find(id) != m_audioSamples.end()) {
		m_audioSamples.erase(id);
	}

	std::unique_ptr<AudioSample> audio = std::make_unique<AudioSample>(id);
	if (path.empty()) {
		if (!audio->CreateSilent()) {
			return false;
		}
	}
	else {
		if (!audio->Create(path)) {
			return false;
		}
	}

	m_audioSamples[id] = std::move(audio);
	*out = m_audioSamples[id].get();

	return true;
}

bool AudioManager::CreateSampleFromData(std::string id, int sampleFlags, int sampleRate, int sampleChannels, int sampleLength, void* sampleData, AudioSample** out) {
	if (m_audioSamples.find(id) != m_audioSamples.end()) {
		m_audioSamples.erase(id);
	}

	std::unique_ptr<AudioSample> audio = std::make_unique<AudioSample>(id);
	if (sampleLength == 0) {
		if (!audio->CreateSilent()) {
			return false;
		}
	} else {
		if (!audio->CreateFromData(sampleFlags, sampleRate, sampleChannels, sampleLength, sampleData)) {
			return false;
		}
	}

	m_audioSamples[id] = std::move(audio);
	*out = m_audioSamples[id].get();

	return true;
}

void AudioManager::Update(double delta) {
	nextUpdate += delta;

	if (nextUpdate >= 0.2) {
		nextUpdate = 0;
		BASS_Update(1000);
	}

	for (auto& [index, audio] : m_audios) {
		audio->Update(delta);
	}
}

Audio* AudioManager::Get(std::string id) {
	if (m_audios.find(id) == m_audios.end()) {
		return nullptr;
	}

	return m_audios[id].get();
}

AudioSample* AudioManager::GetSample(std::string id) {
	if (m_audioSamples.find(id) == m_audioSamples.end()) {
		return nullptr;
	}

	return m_audioSamples[id].get();
}

bool AudioManager::Remove(std::string id) {
	if (m_audios.find(id) == m_audios.end()) {
		return false;
	}

	m_audios.erase(id);

	return true;
}

bool AudioManager::RemoveSample(std::string id) {
	if (m_audioSamples.find(id) == m_audioSamples.end()) {
		return false;
	}

	m_audioSamples.erase(id);
	return true;
}

bool AudioManager::RemoveAll() {
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
