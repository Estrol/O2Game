#include "AudioSample.hpp"
#include <bass.h>

AudioSample::AudioSample(std::string id) {
	m_buffer = nullptr;
	m_size = 0;
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

	if (m_buffer != nullptr) {
		delete[] m_buffer;
	}
}

bool AudioSample::Create(uint8_t* buffer, size_t size) {
	m_buffer = new uint8_t[size];
	memcpy(m_buffer, buffer, size);
	m_size = size;

	m_handle = BASS_SampleLoad(TRUE, m_buffer, 0, size, 10, BASS_SAMPLE_OVER_POS);
	if (!m_handle) {
		return false;
	}
	
	return true;
}

bool AudioSample::Create(std::string path) {
	m_handle = BASS_SampleLoad(FALSE, path.c_str(), 0, 0, 10, BASS_SAMPLE_OVER_POS);
	if (!m_handle) {
		int e = BASS_ErrorGetCode();
		return false;
	}

	return true;
}

bool AudioSample::CreateSilent() {
	m_silent = true;

	return true;
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
