#include "bms.hpp"
#include <iostream>
#include <string>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <numeric>
#include "Util/Util.hpp"

constexpr double EPSILON = 0.0001;

namespace BMS {
	BMSFile::BMSFile() {
		m_lnType = 0;
	}

	BMSFile::~BMSFile() {

	}

	void BMSFile::Load(std::filesystem::path& path) {
		std::fstream fs(path, std::ios::in);
		if (!fs.is_open()) {
			return;
		}

		fs.imbue(std::locale{ ".932" });
		CurrentDir = std::filesystem::path(path).parent_path();

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
					StageFile = mergeVectorWith(data, 1);
					continue;
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
					std::string file = mergeVectorWith(data, 1);

					m_wavs.push_back({ index, file });
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

				if (command.starts_with("LNTYPE")) {
					m_lnType = std::atoi(data[1].c_str());
				}

				if (command.starts_with("LNOBJ")) {
					m_lnObj = data[1];
				}

				// FIELD DATA parsing
				data = splitString(line, ':');
				if (data.size() == 2 && isdigit(data[0][0])) {
					int measure = std::atoi(data[0].substr(0, 3).c_str());
					int channel = std::atoi(data[0].substr(3, 2).c_str());

					if (channel == 0) {
						continue;
					}

					switch (channel) {
						case 2: {
							BMSEvent ev = {};
							ev.Channel = channel;
							ev.Measure = measure;
							ev.Value = std::atof(data[1].c_str());
							ev.Position = 0;

							m_events.push_back(ev);
							break;
						}

						case 3: {
							for (int i = 0; i < data[1].size(); i += 2) {
								std::string value = data[1].substr(i, 2);
								double position = (static_cast<double>(i) / 2.0) / (data[1].size() / 2.0);

								if (value == "00") continue;

								BMSEvent ev = {};
								ev.Channel = channel;
								ev.Measure = measure;
								ev.Position = position;

								try {
									ev.Value = std::stoi(value, nullptr, 16);
								}
								catch (std::invalid_argument&) {
									::printf("[BMS] [ERROR] Failed to parse BPM, undefined behavior may occured!");
									continue;
								}

								m_events.push_back(ev);
							}
							break;
						}

						case 8: {
							for (int i = 0; i < data[1].size(); i += 2) {
								std::string value = data[1].substr(i, 2);
								double position = (static_cast<double>(i) / 2.0) / (data[1].size() / 2.0);

								if (value == "00") continue;

								if (m_bpms.find(value) != m_bpms.end()) {
									BMSEvent ev = {};
									ev.Channel = channel;
									ev.Measure = measure;
									ev.Position = position;
									ev.Value = m_bpms[value];

									m_events.push_back(ev);
								}
							}
							break;
						}

						case 9: {
							for (int i = 0; i < data[1].size(); i += 2) {
								std::string value = data[1].substr(i, 2);
								double position = (static_cast<double>(i) / 2.0) / (data[1].size() / 2.0);

								if (value == "00") continue;

								if (m_stops.find(value) != m_stops.end()) {
									BMSEvent ev = {};
									ev.Channel = channel;
									ev.Measure = measure;
									ev.Position = position;
									ev.Value = m_stops[value];

									m_events.push_back(ev);
								}
							}
							break;
						}

						default: {
							for (int i = 0; i < data[1].size(); i += 2) {
								std::string value = data[1].substr(i, 2);
								double position = (static_cast<double>(i) / 2.0) / (data[1].size() / 2.0);

								if (value == "00") continue;

								BMSEvent ev = {};
								ev.Channel = channel;
								ev.Measure = measure;
								ev.Position = position;
								ev.Value = Base36_Decode(value);

								m_events.push_back(ev);
							}
							break;
						}
					}
					
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

		constexpr auto IsExist = [](const std::vector<int>& vec, int value, int* index) {
			auto it = std::find(vec.begin(), vec.end(), value);
			if (it != vec.end()) {
				*index = it - vec.begin();
				return true;
			}
			else {
				return false;
			}
		};

		Measures.push_back(0);

		BMSTiming startTiming = {};
		startTiming.StartTime = 0;
		startTiming.Value = BPM;
		startTiming.TimeSignature = 1;

		Timings.push_back(startTiming);

		const int BEATS_PER_MSEC = 4 * 60 * 1000;

		double currentBPM = BPM;
		double measureFraction = 1;
		double measurePosition = 0;
		double timer = 0;

		double holdNotes[7] = {};
		int currentMeasure = 0;
		
		std::sort(m_events.begin(), m_events.end(), [](const auto& a, const auto& b) {
			float aPos = (a.Measure + a.Position), bPos = (b.Measure + b.Position);

			// Check if it's STOP channel to prevent note in same position hit after STOP channel.
			if (aPos == bPos) {
				if (a.Channel == 9) return false;
				if (b.Channel == 9) return true;

				return a.Channel < b.Channel;
			}

			return aPos < bPos;
		});

		for (auto& event : m_events) {
			while (event.Measure > currentMeasure) {
				timer += (BEATS_PER_MSEC * (measureFraction - measurePosition)) / currentBPM;
				Measures.push_back(timer);

				currentMeasure++;
				measureFraction = 1;
				measurePosition = 0;
			}

			double position = event.Position * measureFraction;
			timer += (BEATS_PER_MSEC * (position - measurePosition)) / currentBPM;
			measurePosition = position;

			switch (event.Channel) {
				case 1: { // BGM
					BMSAutoSample sample = {};
					sample.SampleIndex = GetSampleIndex(Base36_Encode(static_cast<int>(event.Value)));
					sample.StartTime = timer;

					AutoSamples.push_back(sample);
					break;
				}

				case 2: { // TimeSignature
					measureFraction = event.Value;
					break;
				}

				case 3: // BPM Changes
				case 8: {
					currentBPM = event.Value;

					BMSTiming startTiming = {};
					startTiming.StartTime = timer;
					startTiming.Value = event.Value;
					startTiming.TimeSignature = measureFraction;

					if (currentBPM == 0) {
						::printf("");
					}

					Timings.push_back(startTiming);
					break;
				}

				default: {
					int laneIndex = -1;

					if (IsExist(PlayfieldChannel, event.Channel, &laneIndex)) {
						BMSNote note = {};
						note.StartTime = timer;
						note.EndTime = -1;
						note.Lane = laneIndex;
						note.SampleIndex = GetSampleIndex(Base36_Encode(static_cast<int>(event.Value)));
						
						Notes.push_back(note);
						break;
					}

					if (IsExist(PlayfieldHoldChannel, event.Channel, &laneIndex)) {
						if (holdNotes[laneIndex] == -1) {
							holdNotes[laneIndex] = timer;
						}
						else {
							BMSNote note = {};
							note.StartTime = holdNotes[laneIndex];
							note.EndTime = timer;
							note.Lane = laneIndex;
							note.SampleIndex = GetSampleIndex(Base36_Encode(static_cast<int>(event.Value)));

							holdNotes[laneIndex] = -1;
							Notes.push_back(note);
						}

						break;
					}

					if (IsExist(ScratchChannel, event.Channel, &laneIndex)) {
						BMSAutoSample sample = {};
						sample.SampleIndex = GetSampleIndex(Base36_Encode(static_cast<int>(event.Value)));
						sample.StartTime = timer;

						if (sample.SampleIndex != -1) {
							AutoSamples.push_back(sample);
						}
						break;
					}

					if (event.Channel == 9) {
						BMSTiming startTiming = {};
						startTiming.StartTime = timer;
						startTiming.Value = 0;
						startTiming.TimeSignature = measureFraction;

						double stopTime = (event.Value / 192.0) * BEATS_PER_MSEC / currentBPM;

						BMSTiming endTiming = {};
						endTiming.StartTime = timer + stopTime;
						endTiming.Value = currentBPM;
						endTiming.TimeSignature = measureFraction;

						Timings.push_back(startTiming);
						Timings.push_back(endTiming);

						timer += stopTime;
					}
					break;
				}
			}
		}

		AudioLength = timer + 1000;
	}

	void BMSFile::VerifyNote() {
		for (auto& [index, noteVector] : m_perLaneNotes) {
			for (auto& note : noteVector) {
				Notes.push_back(note);
			}
		}

		std::sort(Notes.begin(), Notes.end(), [](BMSNote& a, BMSNote& b) {
			if (a.StartTime != b.StartTime) {
				return a.StartTime < b.StartTime;
			}
			else {
				return a.Lane < b.Lane;
			}
		});

		std::sort(Timings.begin(), Timings.end(), [](BMSTiming& a, BMSTiming& b) {
			return a.StartTime < b.StartTime;
		});

		std::sort(AutoSamples.begin(), AutoSamples.end(), [](BMSAutoSample& a, BMSAutoSample& b) {
			if (a.StartTime != b.StartTime) {
				return a.StartTime < b.StartTime;
			}
			else {
				return a.SampleIndex < b.SampleIndex;
			}
		});

		for (int i = 0; i < m_wavs.size(); i++) {
			auto& wav = m_wavs[i];
			Samples[i] = wav.second;
		}
	}
	int BMSFile::GetSampleIndex(std::string msg) {
		for (int i = 0; i < m_wavs.size(); i++) {
			if (m_wavs[i].first == msg) {
				return i;
			}
		}

		return -1;
	}
}