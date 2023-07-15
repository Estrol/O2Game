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

    int bufferSize = static_cast<int>(BASS_ChannelSeconds2Bytes(tempoch, 1.0)); // Set buffer size to 1 second
    std::vector<char> dataVec(bufferSize);

    int totalBytesRead = 0;
    char* data = dataVec.data();
    while (true) {
        int bytesRead = BASS_ChannelGetData(tempoch, data + totalBytesRead, bufferSize - totalBytesRead);
        if (bytesRead <= 0) {
            break;
        }

        totalBytesRead += bytesRead;
        if (totalBytesRead >= bufferSize) {
            bufferSize *= 2;
            dataVec.resize(bufferSize);
            data = dataVec.data();
        }
    }

    BASS_ChannelFree(tempoch);
    BASS_ChannelFree(channel);

    size_t size2 = totalBytesRead;
    void* data2 = new char[size2];
    memcpy(data2, dataVec.data(), size2);

    // Return tuple: sampleFlags, sampleRate, sampleChannels, sampleLength, void*
    return { tempoInfo.flags, tempoInfo.freq, tempoInfo.chans, static_cast<int>(size2), data2 };
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

    int bufferSize = static_cast<int>(BASS_ChannelSeconds2Bytes(tempoch, 1.0)); // Set buffer size to 1 second
    std::vector<char> dataVec(bufferSize);

    int totalBytesRead = 0;
    char* data = dataVec.data();
    while (true) {
        int bytesRead = BASS_ChannelGetData(tempoch, data + totalBytesRead, bufferSize - totalBytesRead);
        if (bytesRead <= 0) {
            break;
        }

        totalBytesRead += bytesRead;
        if (totalBytesRead >= bufferSize) {
            bufferSize *= 2;
            dataVec.resize(bufferSize);
            data = dataVec.data();
        }
    }

    BASS_ChannelFree(tempoch);
    BASS_ChannelFree(channel);

    size_t size2 = totalBytesRead;
    void* data2 = new char[size2];
    memcpy(data2, dataVec.data(), size2);

    // Return tuple: sampleFlags, sampleRate, sampleChannels, sampleLength, void*
    return { tempoInfo.flags, tempoInfo.freq, tempoInfo.chans, static_cast<int>(size2), data2 };
}
