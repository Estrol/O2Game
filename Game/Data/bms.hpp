#pragma once
#include <string>
#include <vector>
#include <unordered_map>

namespace BMS {
	struct BMSTiming {
		int section = 0;
		double offset = 0.0;
		double time = 0.0;
		double bpm = -1.0;
		bool changed = false;
		double timeSignature = 1.0;
	};

	struct BMSNote {
		double StartTime;
		double EndTime;
		int Lane;
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
		int Level = 0;
		float BPM = 0.0f;
		float TimeSignature = 0.0f;
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

		BMSNote* m_holdState[7];

		/* idk */
		std::vector<BMSTiming> m_timings;
		std::vector<BMSNote> m_notes;
	};
}