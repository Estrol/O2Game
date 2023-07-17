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

    const int bufferSize = static_cast<int>(size);  // Use the original audio size as the buffer size

    char* data = new char[bufferSize];
    while (true) {
        DWORD read = BASS_ChannelGetData(tempoch, data, bufferSize);
        if (read == -1 || read == 0) {
            break;
        }

        dataVec.insert(dataVec.end(), data, data + read);
    }

    BASS_ChannelFree(tempoch);
    BASS_ChannelFree(channel);

    // Check if less than original size, then append zeros to rest
    if (dataVec.size() < size) {
        dataVec.resize(size, 0);
    }

    delete[] data;
    int size2 = dataVec.size();
    void* data2 = new char[size2];
    memcpy(data2, dataVec.data(), size2);

    // return tuple: sampleFlags, sampleRate, sampleChannels, sampleLength, void*
    return { tempoInfo.flags, tempoInfo.freq, tempoInfo.chans, size2, data2 };
}
