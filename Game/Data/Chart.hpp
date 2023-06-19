#pragma once
#include <iostream>
#include "osu.hpp"
#include "bms.hpp"
#include "OJN.h"
#include <unordered_map>

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

	float Volume = 1, Pan = 0;
};

struct BMSMeasureInfo {
	int Measure;
	int CellSize;
};

struct TimingInfo {
	double StartTime;
	float Value;
	float Beat;
	float TimeSignature;
	TimingType Type;
};

struct Sample {
	std::filesystem::path FileName;
	std::vector<uint8_t> FileBuffer;

	uint32_t Type = 1;
	uint32_t Index;
};

struct AutoSample {
	uint32_t StartTime;
	uint32_t Index;

	float Volume = 1, Pan = 0;
};

enum class Mod {
	MIRROR,
	RANDOM,
	REARRANGE
};

class Chart {
public:
	Chart();
	Chart(Osu::Beatmap& beatmap);
	Chart(BMS::BMSFile& bmsfile);
	Chart(O2::OJN& ojnfile, int diffIndex = 2);
	~Chart();

	void ApplyMod(Mod mod, void* data = NULL);

	float InitialSvMultiplier;
	int m_keyCount;
	float BaseBPM;
	
	int GetLength();
	std::string MD5Hash;

	std::string m_backgroundFile;
	std::vector<char> m_backgroundBuffer;
	std::u8string m_title;
	std::u8string m_artist;
	std::string m_audio;
	std::filesystem::path m_beatmapDirectory;

	std::vector<NoteInfo> m_notes;
	std::vector<TimingInfo> m_bpms;
	std::vector<TimingInfo> m_svs;
	std::vector<double> m_customMeasures;

	std::vector<Sample> m_samples;
	std::vector<AutoSample> m_autoSamples;
private:
	double PredefinedAudioLength = -1;

	void ComputeHash();
	void ComputeKeyCount();

	float GetCommonBPM();
	void NormalizeTimings();
};