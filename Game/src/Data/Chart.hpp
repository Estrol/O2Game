#pragma once
#include <iostream>
#include "osu.hpp"
#include "bms.hpp"
#include "OJN.h"
#include <unordered_map>

// Forward declarations
class AudioSample;

// Note types: NORMAL, HOLD
enum class NoteType : uint8_t {
	// Normal hit note
	NORMAL,

	// Hold note
	HOLD
};

// Timing types: BPM, SV
enum class TimingType : uint8_t {
	// Beat per minute
	BPM,

	// Slider velocity: BPM / 240.0
	SV
};

// Note information
struct NoteInfo {
	double StartTime; // Note start time in milliseconds
	double EndTime; // Note end time in milliseconds
	NoteType Type; // Note type: NORMAL, HOLD
	uint32_t LaneIndex; // Note lane index in range: 0, 1, 2, 3, 4, 5, 6, 7
	uint32_t Keysound; // Keysound index to play in-game

	float Volume = 1, Pan = 0;
};

struct BMSMeasureInfo {
	int Measure;
	int CellSize;
};

struct TimingInfo {
	double StartTime;
	float Value;
	double Beat;
	float TimeSignature;
	TimingType Type;

	double CalculateBeat(double offset);
};

struct Sample {
	std::filesystem::path FileName;
	std::vector<uint8_t> FileBuffer;

	uint32_t Type = 1;
	uint32_t Index;
};

struct AutoSample {
	double StartTime;
	int Index;

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
	
	double GetLength();
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