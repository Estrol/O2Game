#pragma once
#include <iostream>
#include "osu.hpp"
#include <map>

class AudioSample;

enum class NoteType : uint8_t {
	NORMAL,
	HOLD
};

enum class TimingType : uint8_t {
	BPM,
	SV
};

struct NoteInfo {
	uint32_t StartTime;
	uint32_t EndTime;
	NoteType Type;
	uint32_t LaneIndex;
	uint32_t Keysound;
};

struct TimingInfo {
	double StartTime;
	float Value;
	TimingType Type;
};

struct Sample {
	std::string FileName;
};

struct AutoSample {
	uint32_t StartTime;
	std::string FileName;
};

class Chart {
public:
	Chart();
	Chart(Osu::Beatmap& beatmap);
	~Chart();

	float InitialSvMultiplier;
	int m_keyCount;

	std::string m_backgroundFile;
	std::string m_title;
	std::string m_audio;
	std::string m_beatmapDirectory;

	std::vector<NoteInfo> m_notes;
	std::vector<TimingInfo> m_bpms;
	std::vector<TimingInfo> m_svs;

	std::vector<Sample> m_samples;
	std::vector<AutoSample> m_autoSamples;
private:
	float GetCommonBPM();
	void NormalizeTimings();
};