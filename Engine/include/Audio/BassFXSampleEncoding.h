#pragma once
#include <string>
#include <tuple>
#include <vector>

namespace BASS_FX_SampleEncoding {
    struct FXEncoding
    {
        int               sampleFlags;
        int               sampleRate;
        int               sampleChannels;
        int               sampleLength;
        std::vector<char> sampleData;
    };

    // std::tuple<int, int, int, int, void*>
    // sampleFalgs, sampleRate, sampleChannels, sampleLength, void*
    FXEncoding Encode(void *audioData, size_t size, float rate);
    FXEncoding Encode(std::string filePath, float rate);
} // namespace BASS_FX_SampleEncoding