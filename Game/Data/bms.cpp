#include "bms.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>

uint64_t Base36_Decode(const std::string& str) {
	std::string CharList = "0123456789abcdefghijklmnopqrstuvwxyz";
	std::string reversed = str;
	std::reverse(reversed.begin(), reversed.end());
	std::transform(reversed.begin(), reversed.end(), reversed.begin(), ::tolower);

	uint64_t result = 0;
	int pos = 0;

	for (char c : reversed) {
		result += CharList.find(c) * pow(36, pos);
		pos++;
	}

	return result;
}

int BMSTiming_Compare(const BMS::BMSTiming& a, const BMS::BMSTiming& b) {
	if (a.section < b.section) {
		return 1;
	}
	else if (a.section > b.section) {
		return -1;
	}
	else {
		if (a.offset < b.offset) {
			return 1;
		}
		else if (a.offset > b.offset) {
			return -1;
		}
		else {
			return 0;
		}
	}
}

std::vector<std::string> SplitString(const std::string& str, char delimiter) {
	std::vector<std::string> result;

	std::stringstream ss(str);
	std::string line;

	while (std::getline(ss, line, delimiter)) {
		result.push_back(line);
	}

	return result;
}

BMS::BMSFile::BMSFile() {
	for (int i = 0; i < 7; i++) {
		m_holdState[i] = nullptr;
	}
}

BMS::BMSFile::~BMSFile() {

}

bool BMS::BMSFile::IsValid() {
	return m_valid;
}

bool BMS::BMSFile::LoadMetadata(std::vector<std::string>& lines) {
	for (auto& line : lines) {
		auto data = SplitString(line, ' ');
		if (data.size() >= 2) {
			std::string value = "";
			for (int i = 1; i < data.size(); i++) {
				value += data[i] + " ";
			}

			if (data[0] == "#TITLE") {
				Title = value;
			}
			else if (data[0] == "#ARTIST") {
				Artist = value;
			}
			else if (data[0] == "#PLAYLEVEL") {
				Level = std::stoi(value);
			}
			else if (data[0] == "#BPM") {
				BMSTiming timing = {};
				timing.bpm = std::stof(value);

				m_timings.push_back(timing);
			}
			else if (data[0].size() >= 6) {
				if (data[0].substr(0, 4) == "#WAV") {
					std::string index = data[0].substr(4, 2);
					m_wavs[index] = data[1];
				}
				else if (data[0].substr(0, 4) == "#BPM") {
					std::string index = data[0].substr(4, 2);
					m_bpms[index] = std::stof(data[1]);
				}
			}
		}
	}

	return true;
}

bool BMS::BMSFile::LoadTimingField(std::vector<std::string>& lines) {
	for (std::string s : lines) {
		auto data = SplitString(s, ':');

		auto& _time = data[0];
		auto& _note = data[1];

		int measure = std::stoi(_time.substr(1, 3));
		int channel = std::stoi(_time.substr(4, 2));

		switch (channel) {
			case 2: { // BPM Change
				auto exist = std::find_if(m_timings.begin(), m_timings.end(), [measure](BMSTiming& t) {
					return t.section == measure && t.offset == 0 && t.changed == false;
				});

				std::string val_str = _note;
				std::replace(val_str.begin(), val_str.end(), ',', '.');

				if (exist == m_timings.end()) {
					BMSTiming t = {};
					t.timeSignature = std::stof(val_str);
					t.section = measure;
					t.offset = 0;
					t.changed = false;

					m_timings.push_back(t);
				}
				else {
					exist->timeSignature = std::stof(val_str);
				}

				BMSTiming t = {};
				t.section = measure + 1;
				t.offset = 0;
				t.changed = false;
				
				m_timings.push_back(t);
				break;
			}

			case 3: {
				for (int i = 0; i < _note.size(); i += 2) {
					std::string sbpm = _note.substr(i, 2);
					if (sbpm == "00") {
						continue;
					}

					BMSTiming t = {};
					t.section = measure;
					t.offset = 1.0 * i / _note.size();
					t.bpm = std::stoul(sbpm, nullptr, 16);

					m_timings.push_back(t);
				}

				break;
			}

			case 8: {
				for (int i = 0; i < _note.size(); i += 2) {
					std::string index = _note.substr(i, 2);
					if (index == "00") {
						continue;
					}

					

					/*if (std::find(m_bpms.begin(), m_bpms.end(), index) != m_bpms.end()) {
						BMSTiming t = {};
						t.section = measure;
						t.bpm = m_bpms[index];
						t.offset = 1.0 * i / _note.size();

						m_timings.push_back(t);
					}*/
				}

				break;
			}
		}
	}

	return true;
}

bool BMS::BMSFile::LoadNoteData(std::vector<std::string>& lines) {
	for (std::string s : lines) {
		auto data = SplitString(s, ':');

		auto& _time = data[0];
		auto& _note = data[1];

		int measure = std::stoi(_time.substr(1, 3));
		int channel = std::stoi(_time.substr(4, 2));

		const int SCRATCH_NOTE_CHANNEL = 6;
		const int NORMAL_NOTE_CHANNEL = 10;
		const int LONG_NOTE_CHANNEL = 50;
		
		int note_channels[] = { 1, 2, 3, 4, 5, 8, 9 };

		switch (channel) {
			case 1: {
				

				break;
			}

			default: {
				bool Scratch = false;
				bool LN = false;
				int channel = -1;
				int column = 0;

				for (int i = 0; i < 7; i++) {
					column = i;

					if (note_channels[i] + NORMAL_NOTE_CHANNEL == channel) {
						channel = note_channels[i] + NORMAL_NOTE_CHANNEL;
						break;
					}
					else if (note_channels[i] + LONG_NOTE_CHANNEL == channel) {
						channel = note_channels[i] + LONG_NOTE_CHANNEL;
						LN = true;
						break;
					}
				}

				if (channel == -1) {
					continue;
				}

				if (Scratch) {

				}
				else {
					for (int i = 0; i < _note.size(); i += 2) {
						std::string note = _note.substr(i, 2);
						if (note == "00") {
							continue;
						}

						if (LN) {
							if (m_holdState[column] != nullptr) {
								m_holdState[column]->EndTime = GetTimeFromMeasure(measure, 1.0 * i / _note.size());
								m_holdState[column] = nullptr;
							}
							else {
								BMSNote n = {};
								n.SampleIndex = Base36_Decode(note);
								n.Lane = column;
								n.StartTime = GetTimeFromMeasure(measure, 1.0 * i / _note.size());
								m_notes.push_back(n);

								m_holdState[column] = &m_notes.back();
							}
						}
						else {
							BMSNote n = {};
							n.SampleIndex = Base36_Decode(note);
							n.Lane = column;
							n.StartTime = GetTimeFromMeasure(measure, 1.0 * i / _note.size());
							n.EndTime = -1;

							m_notes.push_back(n);
						}
					}
				}
			}
		}
	}

	for (int i = 0; i < 7; i++) {
		if (m_holdState[i] != nullptr) {
			return false;
		}
	}

	return true;
}

void BMS::BMSFile::CalculateTime() {
	BMSTiming current = m_timings[0];

	for (int i = 0; i < m_timings.size(); i++) {
		auto& t = m_timings[i];

		t.time = ((t.section + t.offset) - (current.section + current.offset)) * (60000.0 * 4.0 / current.bpm) * current.timeSignature + current.time;
		current.time = t.time;
		current.section = t.section;
		current.offset = t.offset;
		
		if (t.bpm != -1) {
			current.bpm = t.bpm;
		}
		else {
			t.bpm = current.bpm;
		}

		if (t.timeSignature != current.timeSignature) {
			current.timeSignature = t.timeSignature;
		}
	}
}

double BMS::BMSFile::GetTimeFromMeasure(int measure, double offset) {
	BMSTiming t = {};
	t.section = measure;
	t.offset = offset;

	auto it = std::lower_bound(m_timings.begin(), m_timings.end(), t, BMSTiming_Compare);
	if (it == m_timings.end()) {
		it = m_timings.end() - 1;
	}
	
	return ((t.section + t.offset) - (it->section + it->section)) * (60000.0 * 4.0 / it->bpm) * it->timeSignature * it->time;
}

void BMS::BMSFile::Load(std::string& path) {
	std::vector<std::string> lines;
	
	std::fstream fs(path, std::ios::in);
	if (!fs.is_open()) {
		return;
	}

	std::stringstream _buffer;
	_buffer << fs.rdbuf();
	fs.close();

	std::string buffer = _buffer.str();
	{
		std::stringstream ss(buffer);
		std::string line;

		while (std::getline(ss, line)) {
			if (line.starts_with("#")) {
				if (line.ends_with("\r")) {
					line.pop_back();
				}

				lines.push_back(line);
			}
		}
	}

	LoadMetadata(lines);
	LoadTimingField(lines);

	

	CalculateTime();
	LoadNoteData(lines);

	m_valid = true;
}