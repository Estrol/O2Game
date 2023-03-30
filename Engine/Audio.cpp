#include "Audio.hpp"
#include <iostream>
#include <fstream>

#include <bass.h>
#include <bass_fx.h>

Audio::Audio(std::string id) {
	m_pBuffer = nullptr;
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
	m_hStream = NULL;

	delete[] m_pBuffer;

	return true;
}

bool Audio::Create(std::string fileName) {
	std::fstream fs(fileName, std::ios::binary | std::ios::in);
	if (!fs.is_open()) {
		std::string msg = "Failed to open file: " + fileName;
		MessageBoxA(NULL, msg.c_str(), "EstEngine Error", MB_ICONERROR);

		return false;
	}

	fs.seekg(0, std::ios::end);
	int size = fs.tellg();
	fs.seekg(0, std::ios::beg);

	m_pBuffer = new uint8_t[size];
	fs.read((char*)m_pBuffer, size);
	fs.close();
	
	m_dwSize = size;
	return CreateStream();
}

bool Audio::Create(uint8_t* buffer, size_t size) {
	m_pBuffer = new uint8_t[size];
	memcpy(m_pBuffer, buffer, size);
	m_dwSize = size;

	return CreateStream();
}

bool Audio::Play(DWORD dwStartPosition, BOOL bLoop) {
	if (!m_hStream) {
		return false;
	}

	if (IsPlaying()) {
		Stop();
	}

	if (!BASS_ChannelPlay(m_hStream, bLoop)) {
		int lastErr = BASS_ErrorGetCode();

		return false;
	}

	return true;
}

bool Audio::CreateStream() {
	if (m_hStream) {
		return false;
	}

	m_hStream = BASS_StreamCreateFile(TRUE, m_pBuffer, 0, m_dwSize, BASS_SAMPLE_FLOAT); //BASS_STREAM_DECODE | BASS_SAMPLE_FLOAT);
	if (!m_hStream) {
		MessageBoxA(NULL, "Failed to create stream", "EstEngine Error", MB_ICONERROR);
		return false;
	}

	/*m_hStream = BASS_FX_TempoCreate(m_hStream, BASS_FX_FREESOURCE);
	if (!m_hStream) {
		MessageBoxA(NULL, "Failed to create tempo stream", "EstEngine Error", MB_ICONERROR);
		return false;
	}*/

	volume = 50;
	SetVolume(volume);

	if (!BASS_ChannelUpdate(m_hStream, 0)) {
		MessageBoxA(NULL, "Failed to update stream", "EstEngine Error", MB_ICONERROR);
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

	float frequency = 44100.0f;
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

int Audio::GetDuration() const {
	int length = BASS_ChannelBytes2Seconds(m_hStream, BASS_ChannelGetLength(m_hStream, BASS_POS_BYTE));

	return length;
}

std::string Audio::GetName() const {
	return m_id;
}
