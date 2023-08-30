#include "Audio/BassFXSampleEncoding.h"
#include <iostream>
#include <sstream>
#include <fstream>
#include <bass.h>
#include <bass_fx.h>
#include <vector>
#include <string.h>

BASS_FX_SampleEncoding::FXEncoding BASS_FX_SampleEncoding::Encode(void* audioData, size_t size, float rate) {
	HCHANNEL channel = BASS_StreamCreateFile(TRUE, audioData, 0, size, BASS_STREAM_DECODE);
	if (!channel) {
		return { 0, 0, 0, 0, {} };
	}
	
	int code = BASS_ErrorGetCode();
	HCHANNEL tempoch = BASS_FX_TempoCreate(channel, BASS_STREAM_DECODE);
	if (!tempoch) {
		BASS_ChannelFree(channel);
		return { 0, 0, 0, 0, {} };
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

	// Check if less than 25000 then append zeros to rest if less than
	if (dataVec.size() < 25000) {
		dataVec.resize(25000, 0);
	}

	delete[] data;
	size_t data_size = dataVec.size();
	
	return { 
		(int)tempoInfo.flags, 
		(int)tempoInfo.freq, 
		(int)tempoInfo.chans, 
		(int)data_size,
		std::move(dataVec)
	};
}
