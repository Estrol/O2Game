#include "AudioSample.hpp"
#include <bass.h>
#include <fstream>

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
	m_handle = BASS_SampleLoad(TRUE, buffer, 0, (DWORD)size, 10, BASS_SAMPLE_OVER_POS);
	if (!m_handle) {
		std::cout << "Failed to initialize MEM Sample: " << BASS_ErrorGetCode() << std::endl;
		return false;
	}
	
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

	uint8_t* buffer = new uint8_t[size];
	fs.read((char*)buffer, size);

	m_handle = BASS_SampleLoad(TRUE, buffer, 0, size, 10, BASS_SAMPLE_OVER_POS);
	if (!m_handle) {
		delete[] buffer;

		std::cout << "Failed to initialize FILE Sample: " << BASS_ErrorGetCode() << std::endl;
		return false;
	}

	delete[] buffer;
	return true;
}

bool AudioSample::CreateFromData(int sampleFlags, int sampleRate, int sampleChannels, int sampleLength, void* sampleData) {
	m_handle = BASS_SampleCreate(sampleLength, sampleRate, sampleChannels, 10, BASS_SAMPLE_OVER_POS);
	if (!m_handle) {
		std::cout << "Failed to create blank sample: " << BASS_ErrorGetCode() << std::endl;
		return false;
	}

	bool success = BASS_SampleSetData(m_handle, sampleData);
	if (!success) {
		std::cout << "Failed to set sample data: " << BASS_ErrorGetCode() << std::endl;
		return false;
	}

	delete[] sampleData;
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
