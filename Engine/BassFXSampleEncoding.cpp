#include "BassFXSampleEncoding.hpp"
#include <iostream>
#include <sstream>
#include <fstream>
#include <bass.h>
#include <bass_fx.h>
#include <vector>

std::tuple<int, int, int, int, void*> BASS_FX_SampleEncoding::Encode(void* audioData, size_t size, float rate) {
	HCHANNEL channel = BASS_StreamCreateFile(TRUE, audioData, 0, size, BASS_STREAM_DECODE);
	if (!channel) {
		return { 0, 0, 0, 0, nullptr };
	}
	
	int code = BASS_ErrorGetCode();
	HCHANNEL tempoch = BASS_FX_TempoCreate(channel, BASS_STREAM_DECODE);
	if (!tempoch) {
		BASS_ChannelFree(channel);
		return { 0, 0, 0, 0, nullptr };
	}

	float frequency = 48000.0f;
	BASS_ChannelGetAttribute(tempoch, BASS_ATTRIB_FREQ, &frequency);

	BASS_ChannelSetAttribute(tempoch, BASS_ATTRIB_TEMPO, rate * 100.0f - 100.0f);
	BASS_ChannelSetAttribute(tempoch, BASS_ATTRIB_FREQ, frequency * rate);
	
	BASS_CHANNELINFO tempoInfo;
	BASS_ChannelGetInfo(tempoch, &tempoInfo);
	std::vector<char> dataVec;

	char* data = new char[2048];
	while (true) {
		DWORD read = BASS_ChannelGetData(tempoch, data, 2048);
		if (read == -1 || read == 0) {
			break;
		}

		dataVec.insert(dataVec.end(), data, data + read);
	}

	BASS_ChannelFree(tempoch);
	BASS_ChannelFree(channel);

	// check if less than 25000 then append zeros to rest if less than
	if (dataVec.size() < 25000) {
		dataVec.resize(25000, 0);
	}

	delete[] data;
	size_t size2 = dataVec.size();
	void* data2 = new char[size2];
	memcpy(data2, dataVec.data(), size2);

	// return tuple: sampleFalgs, sampleRate, sampleChannels, sampleLength, void*
	return { tempoInfo.flags, tempoInfo.freq, tempoInfo.chans, (int)size2, data2 };
}

std::tuple<int, int, int, int, void*> BASS_FX_SampleEncoding::Encode(std::string filePath, float rate) {
	HCHANNEL channel = BASS_StreamCreateFile(FALSE, filePath.c_str(), 0, 0, BASS_STREAM_DECODE);
	if (!channel) {
		return { 0, 0, 0, 0, nullptr };
	}

	HCHANNEL tempoch = BASS_FX_TempoCreate(channel, BASS_STREAM_DECODE);
	if (!tempoch) {
		BASS_ChannelFree(channel);
		return { 0, 0, 0, 0, nullptr };
	}

	float frequency = 48000.0f;
	BASS_ChannelGetAttribute(tempoch, BASS_ATTRIB_FREQ, &frequency);

	BASS_ChannelSetAttribute(tempoch, BASS_ATTRIB_TEMPO, rate * 100.0f - 100.0f);
	BASS_ChannelSetAttribute(tempoch, BASS_ATTRIB_FREQ, frequency * rate);

	BASS_CHANNELINFO tempoInfo;
	BASS_ChannelGetInfo(tempoch, &tempoInfo);
	std::vector<char> dataVec;

	char* data = new char[2048];
	while (true) {
		DWORD read = BASS_ChannelGetData(tempoch, data, 2048);
		if (read == -1 || read == 0) {
			break;
		}

		dataVec.insert(dataVec.end(), data, data + read);
	}

	BASS_ChannelFree(tempoch);
	BASS_ChannelFree(channel);

	delete[] data;
	size_t size2 = dataVec.size();
	void* data2 = new char[size2];
	memcpy(data2, dataVec.data(), size2);

	// check if less than 25000 then append zeros to rest if less than
	if (dataVec.size() < 25000) {
		dataVec.resize(25000, 0);
	}

	// return tuple: sampleFalgs, sampleRate, sampleChannels, sampleLength, void*
	return { tempoInfo.flags, tempoInfo.freq, tempoInfo.chans, (int)size2, data2 };
}
