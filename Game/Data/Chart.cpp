#include "chart.hpp"
#include "../../Engine/EstEngine.hpp"
#include <algorithm>
#include <filesystem>

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
					sample.FileName = path;

					m_autoSamples.push_back(sample);
				}
				
				break;
			}
		}
	}

	for (auto& timing : beatmap.TimingPoints) {
		if (timing.BeatLength <= 0) {
			TimingInfo info = {};
			info.StartTime = timing.Offset;
			info.Value = -100.0f / timing.BeatLength;
			info.Type = TimingType::SV;

			m_svs.push_back(info);
		}
		else {
			TimingInfo info = {};
			info.StartTime = timing.Offset;
			info.Value = 60000 / timing.BeatLength;
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

	NormalizeTimings();
}

Chart::~Chart() {
	
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

	std::map<float, int> durations;
	for (int i = (int)m_bpms.size() - 1; i >= 0; i--) {
		auto& tp = m_bpms[i];

		if (tp.StartTime > lastTime) {
			continue;
		}

		int duration = (int)(lastTime - (i == 0 ? 0 : tp.StartTime));
		lastTime = tp.StartTime;

		if (durations.count(tp.Value) == 1) {
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
	double currentSvStartTime = -1.0f;
	double currentSvMultiplier = 1.0f;
	double currentAdjustedSvMultiplier = -1.0f;
	double initialSvMultiplier = -1.0f;

	for (int i = 0; i < m_bpms.size(); i++) {
		auto& tp = m_bpms[i];

		bool hasTimingHaveSameTime = false;
		if (i + 1 < m_bpms.size() && m_bpms[i + 1].StartTime == tp.StartTime) {
			hasTimingHaveSameTime = true;
		}

		while (true) {
			if (currentSvIdx >= m_svs.size()) {
				break;
			}

			auto& sv = m_svs[currentSvIdx];
			if (sv.StartTime > tp.StartTime) {
				break;
			}

			if (hasTimingHaveSameTime && sv.StartTime == tp.StartTime) {
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