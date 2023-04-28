#pragma once
#include <tuple>
#include <string>

namespace BASS_FX_SampleEncoding {
	// std::tuple<int, int, int, int, void*>
	// sampleFalgs, sampleRate, sampleChannels, sampleLength, void*
	std::tuple<int, int, int, int, void*> Encode(void* audioData, size_t size, float rate);
	std::tuple<int, int, int, int, void*> Encode(std::string filePath, float rate);
}