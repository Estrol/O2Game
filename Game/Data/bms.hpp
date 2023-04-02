#pragma once
#include <string>
#include <vector>
#include <map>
#include <unordered_map>

namespace BMS {
	struct BMSRawTiming {
		int section = 0;
		double offset = 0.0;
		double time = 0.0;
		double bpm = -1.0;
		bool changed = true;
		double timeSignature = 1.0;
	};

	struct BMSStopTiming {
		int section = 0;
		double offset = 0.0;
		int Value;
	};

	struct BMSTiming {
		double StartTime;
		double Value;
		float TimeSignature;
	};

	struct BMSNote {
		int StartTime;
		int EndTime;
		int Lane;
		int SampleIndex;
	};

	struct BMSAutoSample {
		int StartTime;
		int SampleIndex;
	};

	class BMSFile {
	public:
		BMSFile();
		~BMSFile();

		void Load(std::string& path);
		bool IsValid();

		std::string Title = "";
		std::string Artist = "";
		std::string FileDirectory = "";
		int Level = 0;
		float BPM = 0.0f;
		float TimeSignature = 0.0f;
		
		std::vector<BMSNote> Notes;
		std::vector<BMSTiming> Timings;
		std::vector<BMSAutoSample> AutoSamples;
		std::map<int, std::string> Samples;

	private:
		bool m_valid = false;

		bool LoadMetadata(std::vector<std::string>& lines);
		bool LoadTimingField(std::vector<std::string>& lines);
		bool LoadNoteData(std::vector<std::string>& lines);
		void CalculateTime();

		double GetTimeFromMeasure(int measure, double offset);

		std::unordered_map<std::string, std::string> m_wavs;
		std::unordered_map<std::string, double> m_bpms;
		std::unordered_map<std::string, bool> m_noteDicts;
		std::unordered_map<std::string, int> m_stops;
		std::vector<BMSRawTiming> m_rawTimings;

		std::vector<BMSStopTiming> m_timingStops;
		BMSNote* m_holdState[8];
	};
}