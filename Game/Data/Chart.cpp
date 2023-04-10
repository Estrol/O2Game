#include "chart.hpp"
#include "../../Engine/EstEngine.hpp"
#include <algorithm>
#include <fstream>
#include <filesystem>
#include "Util/md5.h"

Chart::Chart() {
	InitialSvMultiplier = 1.0f;
	m_keyCount = 7;
}

Chart::Chart(Osu::Beatmap& beatmap) {
	if (!beatmap.IsValid()) {
		throw std::invalid_argument("Invalid osu beatmap!");
	}

	if (beatmap.Mode != 3) {
		throw std::invalid_argument("osu beatmap's Mode must be 3 (or mania mode)");
	}

	if (beatmap.CircleSize < 1 || beatmap.CircleSize > 7) {
		throw std::invalid_argument("osu beatmap's Mania key must be 7");
	}

	m_audio = beatmap.AudioFilename;
	m_title = beatmap.Title;
	m_keyCount = beatmap.CircleSize;
	m_artist = beatmap.Artist;
	m_beatmapDirectory = beatmap.FileDirectory;

	for (auto& event : beatmap.Events) {
		switch (event.Type) {
			case Osu::OsuEventType::Background: {
				std::string fileName = event.params[0];
				fileName.erase(std::remove(fileName.begin(), fileName.end(), '\"'), fileName.end());

				m_backgroundFile = fileName;
				break;
			}

			case Osu::OsuEventType::Sample: {
				std::string fileName = event.params[0];
				fileName.erase(std::remove(fileName.begin(), fileName.end(), '\"'), fileName.end());

				std::string path = beatmap.FileDirectory + "/" + fileName;
				if (std::filesystem::exists(path)) {
					AutoSample sample = {};
					sample.StartTime = event.StartTime;
					sample.Index = beatmap.GetCustomSampleIndex(fileName);

					m_autoSamples.push_back(sample);
				}
				
				break;
			}
		}
	}

	for (auto& note : beatmap.HitObjects) {
		NoteInfo info = {};
		info.StartTime = note.StartTime;
		info.Type = NoteType::NORMAL;
		info.Keysound = note.KeysoundIndex;
		info.LaneIndex = static_cast<int>(std::floorf(note.X * static_cast<float>(beatmap.CircleSize) / 512.0f));

		if (note.Type == 128) {
			info.Type = NoteType::HOLD;
			info.EndTime = note.EndTime;
		}

		m_notes.push_back(info);
	}

	for (auto& timing : beatmap.TimingPoints) {
		bool IsSV = timing.Inherited == 0 || timing.BeatLength < 0;

		if (IsSV) {
			TimingInfo info = {};
			info.StartTime = timing.Offset;
			info.Value = std::clamp(-100.0f / timing.BeatLength, 0.1f, 10.0f);
			info.Type = TimingType::SV;

			m_svs.push_back(info);
		}
		else {
			TimingInfo info = {};
			info.StartTime = timing.Offset;
			info.Value = 60000 / timing.BeatLength;
			info.TimeSignature = timing.TimeSignature;
			info.Type = TimingType::BPM;

			m_bpms.push_back(info);
		}
	}

	auto audioManager = AudioManager::GetInstance();
	for (int i = 0; i < beatmap.HitSamples.size(); i++) {
		auto& keysound = beatmap.HitSamples[i];

		std::string path = beatmap.FileDirectory + "/" + keysound;

		Sample sm = {};
		sm.FileName = path;
		sm.Index = i;
		
		m_samples.push_back(sm);
	}
	
	std::sort(m_autoSamples.begin(), m_autoSamples.end(), [](const AutoSample& a, const AutoSample& b) {
		return a.StartTime < b.StartTime;
	});

	std::sort(m_bpms.begin(), m_bpms.end(), [](const TimingInfo& a, const TimingInfo& b) {
		return a.StartTime < b.StartTime;
	});

	std::sort(m_svs.begin(), m_svs.end(), [](const TimingInfo& a, const TimingInfo& b) {
		return a.StartTime < b.StartTime;
	});

	NormalizeTimings();
	ComputeHash();
}

Chart::Chart(BMS::BMSFile& file) {
	if (!file.IsValid()) {
		throw std::invalid_argument("Invalid BMS file!");
	}

	m_beatmapDirectory = file.FileDirectory;
	m_title = file.Title;
	m_audio = "";// file.FileDirectory + "/" + "test.mp3";
	m_keyCount = 7;
	m_artist = file.Artist;
	m_backgroundFile = file.StageFile;
	BaseBPM = file.BPM;

	int lastTime[7] = {};
	std::sort(file.Notes.begin(), file.Notes.end(), [](const BMS::BMSNote& note1, const BMS::BMSNote note2) {
		return note1.StartTime < note2.StartTime;
	});

	for (auto& note : file.Notes) {
		NoteInfo info = {};
		info.StartTime = note.StartTime;
		info.Type = NoteType::NORMAL;
		info.LaneIndex = note.Lane;
		info.Keysound = note.SampleIndex;

		/*if (note.SampleIndex != -1) {
			AutoSample sm = {};
			sm.StartTime = note.StartTime;
			sm.Index = note.SampleIndex;

			m_autoSamples.push_back(sm);
		}*/
		
		if (note.EndTime != -1) {
			info.Type = NoteType::HOLD;
			info.EndTime = note.EndTime;
		}

		// check if overlap lastTime
		if (info.StartTime < lastTime[info.LaneIndex]) {
			::printf("[Warning] overlapped note found at %d ms and conflict with %d ms\n", info.StartTime, lastTime[info.LaneIndex]);
		}
		else {
			lastTime[info.LaneIndex] = info.StartTime;
			m_notes.push_back(info);
		}
	}

	for (auto& timing : file.Timings) {
		bool IsSV = timing.Value < 0;

		if (IsSV) {
			TimingInfo info = {};
			info.StartTime = timing.StartTime;
			info.Value = std::clamp(timing.Value, 0.1, 10.0);
			info.Type = TimingType::SV;

			m_svs.push_back(info);
		}
		else {
			TimingInfo info = {};
			info.StartTime = timing.StartTime;
			info.Value = timing.Value;
			info.Type = TimingType::BPM;
			info.TimeSignature = 4.0 * timing.TimeSignature;

			m_bpms.push_back(info);
		}
	}

	for (auto& sample : file.Samples) {
		Sample sm = {};
		sm.FileName = file.FileDirectory + "\\" + sample.second;
		sm.Index = sample.first;

		m_samples.push_back(sm);
	}

	for (auto& autoSample : file.AutoSamples) {
		AutoSample sm = {};
		sm.StartTime = autoSample.StartTime;
		sm.Index = autoSample.SampleIndex;

		m_autoSamples.push_back(sm);
	}

	if (m_bpms.size() == 0) {
		TimingInfo info = {};
		info.StartTime = 0;
		info.Value = file.BPM;
		info.Type = TimingType::BPM;

		m_bpms.push_back(info);
	}

	m_bpms[0].Beat = 0;
	for (int i = 1; i < m_bpms.size(); i++) {
		m_bpms[i].Beat = m_bpms[i - 1].Beat + (m_bpms[i].StartTime - m_bpms[i - 1].StartTime) * (m_bpms[i - 1].Value / 60000.0f);
	}

	std::sort(m_autoSamples.begin(), m_autoSamples.end(), [](const AutoSample& a, const AutoSample& b) {
		return a.StartTime < b.StartTime;
	});

	std::sort(m_bpms.begin(), m_bpms.end(), [](const TimingInfo& a, const TimingInfo& b) {
		return a.StartTime < b.StartTime;
	});

	std::sort(m_svs.begin(), m_svs.end(), [](const TimingInfo& a, const TimingInfo& b) {
		return a.StartTime < b.StartTime;
	});

	NormalizeTimings();
	ComputeHash();
}

Chart::~Chart() {
	
}

int Chart::GetLength() {
	int length = 0;

	for (auto& note : m_notes) {
		if (note.StartTime > length) {
			length = note.StartTime;
		}

		if (note.EndTime > length) {
			length = note.EndTime;
		}
	}

	return length;
}

void Chart::ComputeHash() {
	std::string result;
	for (int i = 0; i < m_notes.size(); i++) {
		result += std::to_string(m_notes[i].StartTime + m_notes[i].EndTime);
	}

	uint8_t data[16];
	md5String((char*)result.c_str(), data);

	std::stringstream ss;
	ss << std::hex << std::setfill('0');
	for (int i = 0; i < 16; i++) {
		ss << std::setw(2) << static_cast<int>(data[i]);
	}

	MD5Hash = ss.str();
	std::cout << "Map hash: " << MD5Hash << std::endl;
}

float Chart::GetCommonBPM() {
	if (m_bpms.size() == 0) {
		return 0.0f;
	}

	std::vector<NoteInfo> orderedByDescending(m_notes);
	std::sort(orderedByDescending.begin(), orderedByDescending.end(), [](const NoteInfo& a, const NoteInfo& b) {
		return a.StartTime > b.StartTime;
	});

	auto& lastObject = orderedByDescending[0];
	double lastTime = lastObject.Type == NoteType::HOLD ? lastObject.EndTime : lastObject.StartTime;

	std::unordered_map<float, int> durations;
	for (int i = (int)m_bpms.size() - 1; i >= 0; i--) {
		auto& tp = m_bpms[i];

		if (tp.StartTime > lastTime) {
			continue;
		}

		int duration = (int)(lastTime - (i == 0 ? 0 : tp.StartTime));
		lastTime = tp.StartTime;

		if (durations.find(tp.Value) != durations.end()) {
			durations[tp.Value] += duration;
		}
		else {
			durations[tp.Value] = duration;
		}
	}

	if (durations.size() == 0) {
		return m_bpms[0].Value;
	}

	int currentDuration = 0;
	float currentBPM = 0.0f;

	for (auto& [bpm, duration] : durations) {
		if (duration > currentDuration) {
			currentDuration = duration;
			currentBPM = bpm;
		}
	}

	return currentBPM;
}

void Chart::NormalizeTimings() {
	std::vector<TimingInfo> result;

	float baseBPM = GetCommonBPM();
	float currentBPM = m_bpms[0].Value;
	int currentSvIdx = 0;

	// Ambigous
	BaseBPM = baseBPM;

	double currentSvMultiplier = 1.0f;

	double currentSvStartTime = -1.0f;
	double currentAdjustedSvMultiplier = -1.0f;
	double initialSvMultiplier = -1.0f;

	for (int i = 0; i < m_bpms.size(); i++) {
		auto& tp = m_bpms[i];

		bool exist = false;
		if ((i + 1) < m_bpms.size() && m_bpms[i + 1].StartTime == tp.StartTime) {
			exist = true;
		}

		while (true) {
			if (currentSvIdx >= m_svs.size()) {
				break;
			}

			auto& sv = m_svs[currentSvIdx];
			if (sv.StartTime > tp.StartTime) {
				break;
			}

			if (exist && sv.StartTime == tp.StartTime) {
				break;
			}

			if (sv.StartTime < tp.StartTime) {
				float multiplier = sv.Value * (currentBPM / baseBPM);

				if (currentAdjustedSvMultiplier == -1.0f) {
					currentAdjustedSvMultiplier = multiplier;
					initialSvMultiplier = multiplier;
				}

				if (multiplier != currentAdjustedSvMultiplier) {
					TimingInfo info = {};
					info.StartTime = sv.StartTime;
					info.Value = multiplier;
					info.Type = TimingType::SV;

					result.push_back(info);
					currentAdjustedSvMultiplier = multiplier;
				}
			}

			currentSvStartTime = sv.StartTime;
			currentSvMultiplier = sv.Value;
			currentSvIdx += 1;
		}

		if (currentSvStartTime == -1.0f || currentSvStartTime < tp.StartTime) {
			currentSvMultiplier = 1.0f;
		}

		currentBPM = tp.Value;

		float multiplier = currentSvMultiplier * (currentBPM / baseBPM);

		if (currentAdjustedSvMultiplier == -1.0f) {
			currentAdjustedSvMultiplier = multiplier;
			initialSvMultiplier = multiplier;
		}

		if (multiplier != currentAdjustedSvMultiplier) {
			TimingInfo info = {};
			info.StartTime = tp.StartTime;
			info.Value = multiplier;
			info.Type = TimingType::SV;

			result.push_back(info);
			currentAdjustedSvMultiplier = multiplier;
		}
	}

	for (; currentSvIdx < m_svs.size(); currentSvIdx++) {
		auto& sv = m_svs[currentSvIdx];
		float multiplier = sv.Value * (currentBPM / baseBPM);

		if (multiplier != currentAdjustedSvMultiplier) {
			TimingInfo info = {};
			info.StartTime = sv.StartTime;
			info.Value = multiplier;
			info.Type = TimingType::SV;

			result.push_back(info);
			currentAdjustedSvMultiplier = multiplier;
		}
	}

	InitialSvMultiplier = initialSvMultiplier == -1.0f ? 1 : initialSvMultiplier;

	m_svs.clear();
	m_svs = result;
}