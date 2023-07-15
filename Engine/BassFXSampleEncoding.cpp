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

    // Calculate buffer size based on input size and desired quality
    size_t bufferSize = size * static_cast<size_t>(rate);

    void* data = new char[bufferSize];
    DWORD bytesRead = 0;
    size_t totalBytesRead = 0;
    while (totalBytesRead < bufferSize) {
        bytesRead = BASS_ChannelGetData(tempoch, static_cast<char*>(data) + totalBytesRead, bufferSize - totalBytesRead);
        if (bytesRead <= 0) {
            break;
        }
        totalBytesRead += bytesRead;
    }

    BASS_ChannelFree(tempoch);
    BASS_ChannelFree(channel);

    // Return tuple: sampleFlags, sampleRate, sampleChannels, sampleLength, void*
    return { tempoInfo.flags, tempoInfo.freq, tempoInfo.chans, static_cast<int>(totalBytesRead), data };
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

    // Get input file size
    std::ifstream file(filePath, std::ios::binary | std::ios::ate);
    if (!file) {
        BASS_ChannelFree(tempoch);
        BASS_ChannelFree(channel);
        return { 0, 0, 0, 0, nullptr };
    }
    std::streamsize fileSize = file.tellg();
    file.close();

    // Calculate buffer size based on input size and desired quality
    size_t bufferSize = static_cast<size_t>(fileSize) * static_cast<size_t>(rate);

    void* data = new char[bufferSize];
    DWORD bytesRead = 0;
    size_t totalBytesRead = 0;
    while (totalBytesRead < bufferSize) {
        bytesRead = BASS_ChannelGetData(tempoch, static_cast<char*>(data) + totalBytesRead, bufferSize - totalBytesRead);
        if (bytesRead <= 0) {
            break;
        }
        totalBytesRead += bytesRead;
    }

    BASS_ChannelFree(tempoch);
    BASS_ChannelFree(channel);

    // Return tuple: sampleFlags, sampleRate, sampleChannels, sampleLength, void*
    return { tempoInfo.flags, tempoInfo.freq, tempoInfo.chans, static_cast<int>(totalBytesRead), data };
}
