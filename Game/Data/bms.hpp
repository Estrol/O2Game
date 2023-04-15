#pragma once
#include <string>
#include <vector>
#include <map>
#include <unordered_map>

namespace BMS {
	struct BMSRawTiming;
	struct BMSStop;
	struct BMSEvent;

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
		std::string StageFile = "";
		int Level = 0;
		float BPM = 130.0f;
		float TimeSignature = 0.0f;
		
		std::vector<BMSNote> Notes;
		std::vector<BMSTiming> Timings;
		std::vector<BMSAutoSample> AutoSamples;
		std::map<int, std::string> Samples;

	private:
		void CompileData(std::vector<std::string>& lines);
		void CompileNoteData();
		void VerifyNote();
		int GetSampleIndex(std::string msg);

		bool m_valid = false;
		std::string m_lnObj;
		int m_lnType;
		
		std::map<int, std::vector<BMSNote>> m_perLaneNotes;
		std::vector<std::pair<std::string, std::string>> m_wavs;
		std::unordered_map<std::string, double> m_bpms;
		std::unordered_map<std::string, double> m_stops;
		std::map<int, std::vector<BMSEvent>> m_events;
	};
}