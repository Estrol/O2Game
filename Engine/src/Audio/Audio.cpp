#include "Audio/Audio.h"
#include <iostream>
#include <fstream>
#include <thread>
#include "MsgBox.h"
#include <cmath>

#include <bass.h>
#include <bass_fx.h>
#include <string.h>

Audio::Audio(std::string id) {
	m_mutex = std::make_unique<std::mutex>();
	m_vBuffer = {};
	m_dwSize = 0;
	m_hStream = 0;

	m_type = AudioType::STREAM;
	m_id = id;
}

Audio::~Audio() {
	Release();
}

AudioType Audio::GetType() const {
	return m_type;
}

bool Audio::Release() {
	if (m_hStream == NULL) {
		return true;
	}

	if (IsPlaying()) {
		Stop();
	}

	BASS_StreamFree(m_hStream);
	m_hStream = 0;

	m_vBuffer.clear();
	return true;
}

bool Audio::Create(std::filesystem::path fileName) {
	std::fstream fs(fileName, std::ios::binary | std::ios::in);
	if (!fs.is_open()) {
		std::string msg = "Failed to open file: " + fileName.string();
		MsgBox::ShowOut(msg, "EstEngine Error", MsgBoxType::OK, MsgBoxFlags::BTN_ERROR);

		return false;
	}

	fs.seekg(0, std::ios::end);
	size_t size = fs.tellg();
	fs.seekg(0, std::ios::beg);

	m_vBuffer.resize(size);
	fs.read((char*)m_vBuffer.data(), size);
	fs.close();
	
	m_dwSize = size;
	return CreateStream();
}

bool Audio::Create(uint8_t* buffer, size_t size) {
	m_vBuffer.resize(size);
	memcpy(m_vBuffer.data(), buffer, size);
	m_dwSize = size;

	return CreateStream();
}

bool Audio::Play(uint32_t dwStartPosition, bool bLoop) {
	if (!m_hStream) {
		return false;
	}

	if (IsPlaying()) {
		Stop();
	}

	if (bLoop) {
		BASS_ChannelFlags(m_hStream, BASS_SAMPLE_LOOP, BASS_SAMPLE_LOOP);
	}
	else {
		BASS_ChannelFlags(m_hStream, 0, BASS_SAMPLE_LOOP);
	}

	if (!BASS_ChannelPlay(m_hStream, TRUE)) {
		int lastErr = BASS_ErrorGetCode();

		return false;
	}

	return true;
}

bool Audio::CreateStream() {
	if (m_hStream) {
		return false;
	}

	m_hStream = BASS_StreamCreateFile(TRUE, m_vBuffer.data(), 0, m_dwSize, BASS_STREAM_DECODE | BASS_SAMPLE_FLOAT);
	if (!m_hStream) {
		std::cout << "BASS_ERROR: " << BASS_ErrorGetCode() << std::endl;
		MsgBox::ShowOut("EstEngine Error", "Failed to create Stream", MsgBoxType::OK, MsgBoxFlags::BTN_ERROR);
		return false;
	}

	m_hStream = BASS_FX_TempoCreate(m_hStream, BASS_FX_FREESOURCE);
	if (!m_hStream) {
		std::cout << "BASS_ERROR: " << BASS_ErrorGetCode() << std::endl;
		MsgBox::ShowOut("EstEngine Error", "Failed to create Tempo Stream", MsgBoxType::OK, MsgBoxFlags::BTN_ERROR);
		return false;
	}

	volume = 50;
	SetVolume(volume);

	if (!BASS_ChannelUpdate(m_hStream, 1000)) {
		MsgBox::ShowOut("Failed to update stream", "EstEngine Error", MsgBoxType::OK, MsgBoxFlags::BTN_ERROR);
		return false;
	}

	return true;
}

bool Audio::IsPlaying() {
	if (!m_hStream) {
		return false;
	}

	return BASS_ChannelIsActive(m_hStream) == BASS_ACTIVE_PLAYING;
}

bool Audio::IsFadeOut() {
	return is_fade_rn;
}

bool Audio::FadeIn() {
	if (!m_hStream) {
		return false;
	}

	is_fade_rn = false;
	FadeStream(volume, false);

	return true;
}

bool Audio::FadeOut() {
	if (!m_hStream) {
		return false;
	}

	is_fade_rn = true;
	FadeStream(volume, true);

	return true;
}

void Audio::FadeStream(int volume, bool state) {
	std::thread tr = std::thread([=] {
		std::lock_guard<std::mutex> _lock(*m_mutex.get());

		float initialVolume = state ? (float)volume / 100.0f : 0.0f;
		float targetVolume = state ? 0.0f : (float)volume / 100.0f;

		BASS_ChannelSlideAttribute(m_hStream, BASS_ATTRIB_VOL, targetVolume, 1000);

		auto fadeStartTime = std::chrono::steady_clock::now();
		auto fadeEndTime = fadeStartTime + std::chrono::milliseconds(1000);

		while (std::chrono::steady_clock::now() < fadeEndTime) {
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
		}

		BASS_ChannelSetAttribute(m_hStream, BASS_ATTRIB_VOL, targetVolume);
	});

	tr.detach();
}

void Audio::Update() {
	
}

bool Audio::Pause() {
	if (!m_hStream) {
		return false;
	}

	if (!BASS_ChannelPause(m_hStream)) {
		return false;
	}

	return true;
}

bool Audio::Resume() {
	if (!m_hStream) {
		return false;
	}

	if (BASS_ChannelPause(m_hStream) == FALSE) {
		return false;
	}

	if (BASS_ChannelSetPosition(m_hStream, 0, 0) == FALSE) {
		return false;
	}

	return true;
}

bool Audio::Stop() {
	if (!m_hStream) {
		return false;
	}

	bool result = BASS_ChannelStop(m_hStream);
	if (!result) {
		return false;
	}

	return true;
}

void Audio::SetVolume(int vol) {
	volume = vol;
	BASS_ChannelSetAttribute(m_hStream, BASS_ATTRIB_VOL, (float)vol / 100.0f);
}

void Audio::SetPan(int _pan) {
	pan = _pan;
	BASS_ChannelSetAttribute(m_hStream, BASS_ATTRIB_PAN, (float)pan / 100.0f);
}

void Audio::SetRate(float _rate) {
	rate = _rate;

	float frequency = 48000.0f;
	BASS_ChannelGetAttribute(m_hStream, BASS_ATTRIB_FREQ, &frequency);

	if (pitch) {
		BASS_ChannelSetAttribute(m_hStream, BASS_ATTRIB_TEMPO, 0);
		BASS_ChannelSetAttribute(m_hStream, BASS_ATTRIB_FREQ, frequency * rate);
	}
	else {
		BASS_ChannelSetAttribute(m_hStream, BASS_ATTRIB_TEMPO, (rate * 100) - 100);
		BASS_ChannelSetAttribute(m_hStream, BASS_ATTRIB_FREQ, frequency);
	}
}

void Audio::SetPitch(bool enabled) {
	pitch = enabled;
	SetRate(rate);
}

#if __GCC__
double round(double value) {
	return value < 0.0 ? ceil(value - 0.5) : floor(value + 0.5);
}
#endif

int Audio::GetDuration() const {
	double length = BASS_ChannelBytes2Seconds(m_hStream, BASS_ChannelGetLength(m_hStream, BASS_POS_BYTE));

#if __GCC__
	return static_cast<int>(round(length));
#else
	return static_cast<int>(std::round(length));
#endif
}

std::string Audio::GetName() const {
	return m_id;
}
