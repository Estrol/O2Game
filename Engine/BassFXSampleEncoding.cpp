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

    HCHANNEL tempoch = BASS_FX_TempoCreate(channel, BASS_STREAM_DECODE);
    if (!tempoch) {
        BASS_ChannelFree(channel);
        return { 0, 0, 0, 0, nullptr };
    }

    float frequency = 48000.0f;
    BASS_ChannelGetAttribute(tempoch, BASS_ATTRIB_FREQ, &frequency);

    BASS_ChannelSetAttribute(tempoch, BASS_ATTRIB_TEMPO, rate * 100.0f - 100.0f);
    BASS_ChannelSetAttribute(tempoch, BASS_ATTRIB_FREQ, frequency * rate);
    BASS_ChannelSetAttribute(tempoch, BASS_ATTRIB_SRC, 4); // Set resampling quality to best

    BASS_CHANNELINFO tempoInfo;
    BASS_ChannelGetInfo(tempoch, &tempoInfo);
    std::vector<char> dataVec;

    // Determine a suitable buffer size based on the audio size
    const size_t bufferSize = size; // Adjust the divisor according to your needs (or you can divide it bufferSize = size / 10 but i don't see improvement)

    char* data = new char[bufferSize];
    DWORD read = 0;

    while ((read = BASS_ChannelGetData(tempoch, data, bufferSize)) > 0) {
        dataVec.insert(dataVec.end(), data, data + read);
    }

    BASS_ChannelFree(tempoch);
    BASS_ChannelFree(channel);

    delete[] data;

    // Resize to the actual data size and fill with zeros if necessary
    const size_t size2 = dataVec.size();
    if (size2 < size) {
        dataVec.resize(size);
        std::fill(dataVec.begin() + size2, dataVec.end(), 0);
    }

    void* data2 = new char[size];
    memcpy(data2, dataVec.data(), size);

    // Return tuple: sampleFlags, sampleRate, sampleChannels, sampleLength, void*
    return { tempoInfo.flags, tempoInfo.freq, tempoInfo.chans, static_cast<int>(size), data2 };
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
    BASS_ChannelSetAttribute(tempoch, BASS_ATTRIB_SRC, 4); // Set resampling quality to best

    BASS_CHANNELINFO tempoInfo;
    BASS_ChannelGetInfo(tempoch, &tempoInfo);
    std::vector<char> dataVec;

    // Determine a suitable buffer size based on the audio length
    const double duration = BASS_ChannelBytes2Seconds(tempoch, BASS_ChannelGetLength(tempoch, BASS_POS_BYTE));
    const size_t bufferSize = static_cast<size_t>(duration * frequency) * sizeof(float);

    char* data = new char[bufferSize];
    DWORD read = 0;

    while ((read = BASS_ChannelGetData(tempoch, data, bufferSize)) > 0) {
        dataVec.insert(dataVec.end(), data, data + read);
    }

    BASS_ChannelFree(tempoch);
    BASS_ChannelFree(channel);

    delete[] data;

    // Resize to the actual data size and fill with zeros if necessary
    const size_t size2 = dataVec.size();
    if (size2 < bufferSize) {
        dataVec.resize(bufferSize);
        std::fill(dataVec.begin() + size2, dataVec.end(), 0);
    }

    void* data2 = new char[bufferSize];
    memcpy(data2, dataVec.data(), bufferSize);

    // Return tuple: sampleFlags, sampleRate, sampleChannels, sampleLength, void*
    return { tempoInfo.flags, tempoInfo.freq, tempoInfo.chans, static_cast<int>(bufferSize), data2 };
}
