#include "Audio/AudioSample.h"
#include <bass.h>
#include <bass_fx.h>
#include <fstream>
#include <string.h>
#include <vector>

AudioSample::AudioSample(std::string id) {
	m_silent = false;
	m_handle = NULL;
	m_rate = 1.0;
	m_vol = 50;
	m_pitch = FALSE;

	m_id = id;
}

AudioSample::~AudioSample() {
	if (m_silent) {
		return;
	}

	if (m_handle != NULL) {
		BASS_SampleFree(m_handle);
	}
}

bool AudioSample::Create(uint8_t* buffer, size_t size) {
	std::vector<uint8_t> data(size);
	memcpy(data.data(), buffer, size);

	m_handle = BASS_SampleLoad(TRUE, data.data(), 0, (DWORD)size, 10, BASS_SAMPLE_OVER_POS);
	if (!m_handle) {
		std::cout << "Failed to initialize MEM Sample: " << BASS_ErrorGetCode() << std::endl;
		return false;
	}

	CheckAudioTime();
	return true;
}

bool AudioSample::Create(std::filesystem::path path) {
	std::fstream fs(path, std::ios::binary | std::ios::in);
	if (!fs.is_open()) {
		std::cout << "Failed to open file: " << path << std::endl;
		return false;
	}

	fs.seekg(0, std::ios::end);
	size_t size = fs.tellg();
	fs.seekg(0, std::ios::beg);

	std::vector<uint8_t> buffer(size);
	fs.read((char*)buffer.data(), size);

	m_handle = BASS_SampleLoad(TRUE, buffer.data(), 0, (DWORD)size, 10, BASS_SAMPLE_OVER_POS | BASS_MUSIC_PRESCAN);
	if (!m_handle) {
		std::cout << "Failed to initialize FILE Sample: " << BASS_ErrorGetCode() << std::endl;
		return false;
	}

	CheckAudioTime();
	return true;
}

bool AudioSample::CreateFromData(int sampleFlags, int sampleRate, int sampleChannels, int sampleLength, void* sampleData) {
	m_handle = BASS_SampleCreate(sampleLength, sampleRate, sampleChannels, 10, BASS_MUSIC_PRESCAN | BASS_SAMPLE_OVER_POS | sampleFlags);
	if (!m_handle) {
		std::cout << "Failed to create blank sample: " << BASS_ErrorGetCode() << std::endl;
		return false;
	}

	std::vector<uint8_t> data(sampleLength);
	memcpy(data.data(), sampleData, sampleLength);

	bool success = BASS_SampleSetData(m_handle, reinterpret_cast<void*>(data.data()));
	if (!success) {
		std::cout << "Failed to set sample data: " << BASS_ErrorGetCode() << std::endl;
		return false;
	}

	CheckAudioTime();
	return true;
}

bool AudioSample::CreateSilent() {
	m_silent = true;

	return true;
}

void AudioSample::SetRate(double rate) {
	m_rate = static_cast<float>(rate);
}

std::string AudioSample::GetId() const {
	return m_id;
}

std::unique_ptr<AudioSampleChannel> AudioSample::CreateChannel() {
	if (m_silent) {
		return std::make_unique<AudioSampleChannel>();
	}

	return std::make_unique<AudioSampleChannel>(m_handle, m_rate, m_vol, m_pitch);
}

void AudioSample::CheckAudioTime() {
	QWORD length = BASS_ChannelGetLength(m_handle, BASS_POS_BYTE);
	double ms = BASS_ChannelBytes2Seconds(m_handle, length) * 1000;

	if (ms < 25) {
		
	}
}
