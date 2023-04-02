#include "bms.hpp"
#include "bms_struct.hpp"
#include "bms_calculation.hpp"

#include <iostream>
#include <string>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <numeric>
#include "Util/Util.hpp"

namespace BMS {
	BMSFile::BMSFile() {

	}

	BMSFile::~BMSFile() {

	}

	void BMSFile::Load(std::string& path) {
		std::fstream fs(path, std::ios::in);
		if (!fs.is_open()) {
			return;
		}

		FileDirectory = std::filesystem::path(path).parent_path().string();

		std::stringstream ss;
		ss << fs.rdbuf();
		fs.close();

		auto lines = splitString(ss);
		CompileData(lines);
		CompileNoteData();
		VerifyNote();

		m_valid = true;
	}

	bool BMSFile::IsValid() {
		return m_valid;
	}

	void BMSFile::CompileData(std::vector<std::string>& lines) {
		m_events = {};

		for (std::string line : lines) {
			if (line.starts_with("#")) {
				line = removeComment(line);

				auto data = splitString(line, ' ');
				std::string command = data[0];
				std::transform(command.begin(), command.end(), command.begin(), ::toupper);

				if (command.starts_with("PLAYER")) {
					try {
						if (data.size() == 2) {
							switch (std::atoi(data[1].c_str())) {
								case 1: {
									break;
								}

								default: {
									::printf("[BMS] [WARNING] Unsupported #PLAYER file, some notes might not able to load it\n");
									break;
								}
							}
						}
					}
					catch (std::invalid_argument& e) {
						::printf("[BMS] [ERROR] Failed to parse #PLAYER, the notes might not able correctly load\n");
					}

					continue;
				}

				if (command.starts_with("TITLE")) {
					Title = mergeVectorWith(data, 1);
					continue;
				}

				if (command.starts_with("ARTIST")) {
					Artist = mergeVectorWith(data, 1);
					continue;
				}

				if (command.starts_with("STAGEFILE")) {
					continue; // TODO:
				}

				if (command.starts_with("BPM") 
					&& command.size() == 3) {
					BPM = std::atof(mergeVector(data, 1).c_str());
					continue;
				}

				// WAVS, BPM and STOP parsing
				if (command.starts_with("WAV") 
					&& command.size() == 5) {
					std::string index = command.substr(3, 2);
					std::string file = mergeVector(data, 1);

					m_wavs[index] = file;
					continue;
				}

				if (command.starts_with("BPM") 
					&& command.size() == 5) {
					std::string index = command.substr(3, 2);
					double bpm = std::atof(mergeVector(data, 1).c_str());

					m_bpms[index] = bpm;
					continue;
				}

				if (command.starts_with("STOP")) {
					std::string index = command.substr(4, 2);
					double duration = std::atof(mergeVector(data, 1).c_str());

					m_stops[index] = duration;
					continue;
				}

				// FIELD DATA parsing
				data = splitString(line, ':');
				if (data.size() == 2 && isdigit(data[0][0])) {
					int measure = std::atoi(data[0].substr(0, 3).c_str());
					int channel = std::atoi(data[0].substr(3, 2).c_str());

					if (channel == 0) {
						continue;
					}

					std::vector<std::string> params;
					for (int i = 0; i < data[1].size(); i += 2) {
						params.push_back(data[1].substr(i, 2));
					}

					BMSEvent ev = {};
					ev.Channel = channel;
					ev.Measure = measure;
					ev.Params = params;

					if (m_events.find(measure) == m_events.end()) {
						m_events[measure] = {};
					}

					m_events[measure].push_back(ev);
					continue;
				}
			}
		}
	}

	void BMSFile::CompileNoteData() {
		const std::vector<int> PlayfieldChannel = { 11, 12, 13, 14, 15, 18, 19 };
		const std::vector<int> PlayfieldHoldChannel = { 51, 52, 53, 54, 55, 58, 59 };
		const std::vector<int> ScratchChannel = { 16, 56 };
		BMSNote* PendingHold[7] = {};

		constexpr auto isExist = [](const std::vector<int>& vec, int value, int* index) {
			auto it = std::find(vec.begin(), vec.end(), value);
			if (it != vec.end()) {
				*index = it - vec.begin();
				return true;
			}
			else {
				return false;
			}
		};

		double startOffset = 0.0;
		double initBPM = BPM;

		// measure
		for (auto& track : m_events) {
			Calculation::Timing t = { track.second, m_bpms, m_stops };

			for (auto& event : track.second) {
				int laneIndex = -1;

				// Normal Note
				if (isExist(PlayfieldChannel, event.Channel, &laneIndex)) {
					for (int i = 0; i < event.Params.size(); i++) {
						auto& msg = event.Params[i];
						if (msg == "00") continue;

						BMSNote note = {};
						note.StartTime = startOffset + t.GetStartTimeFromOffset(initBPM, 1.0 * i / event.Params.size(), false);
						note.EndTime = -1;
						note.Lane = laneIndex;
						note.SampleIndex = Base36_Decode(msg);

						Notes.push_back(note);
					}
				}

				// Hold Channel
				if (isExist(PlayfieldHoldChannel, event.Channel, &laneIndex)) {
					for (int i = 0; i < event.Params.size(); i++) {
						auto& msg = event.Params[i];
						if (msg == "00") continue;

						if (PendingHold[laneIndex] != nullptr) {
							PendingHold[laneIndex]->EndTime = startOffset + t.GetStartTimeFromOffset(initBPM, 1.0 * i / event.Params.size(), false);
							PendingHold[laneIndex] = nullptr;
						}
						else {
							BMSNote note = {};
							note.StartTime = startOffset + t.GetStartTimeFromOffset(initBPM, 1.0 * i / event.Params.size(), false);
							note.EndTime = -1;
							note.Lane = laneIndex;
							note.SampleIndex = Base36_Decode(msg);

							Notes.push_back(note);
						}
					}
				}

				// Scratch Channel -> BGM
				if (isExist(ScratchChannel, event.Channel, &laneIndex) || event.Channel == 1) {
					for (int i = 0; i < event.Params.size(); i++) {
						auto& msg = event.Params[i];
						if (msg == "00") continue;

						if (m_wavs.find(msg) != m_wavs.end()) {
							BMSAutoSample at = {};
							at.StartTime = startOffset + t.GetStartTimeFromOffset(initBPM, 1.0 * i / event.Params.size(), false);
							at.SampleIndex = Base36_Decode(msg);

							AutoSamples.push_back(at);
						}
					}
				}
			}

			double trackDuration = t.GetStartTimeFromOffset(initBPM, 1);
			auto timings = t.GetTimings(startOffset, initBPM);
			if (timings.size() > 0) {
				auto it = timings.end() - 1;
				initBPM = it->second;

				for (auto& it : timings) {
					BMSTiming t = {};
					t.StartTime = it.first;
					t.Value = it.second;
					t.TimeSignature = 4.0 / 4.0;

					Timings.push_back(t);
				}
			}

			startOffset += trackDuration;
		}
	}

	void BMSFile::VerifyNote() {
		std::sort(Notes.begin(), Notes.end(), [](BMSNote& a, BMSNote& b) {
			return a.StartTime < b.StartTime;
		});

		std::sort(Timings.begin(), Timings.end(), [](BMSTiming& a, BMSTiming& b) {
			return a.StartTime < b.StartTime;
		});

		for (auto& sample : m_wavs) {
			int index = Base36_Decode(sample.first);
			Samples[index] = sample.second;
		}
	}
}