#include "chart.hpp"
#include "../../Engine/EstEngine.hpp"
#include <algorithm>
#include <fstream>
#include <filesystem>
#include "Util/md5.h"
#include <random>
#include "Util/Util.hpp"

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

	//m_audio = beatmap.AudioFilename;
	m_title = std::u8string(beatmap.Title.begin(), beatmap.Title.end());
	m_keyCount = beatmap.CircleSize;
	m_artist = std::u8string(beatmap.Artist.begin(), beatmap.Artist.end());
	m_beatmapDirectory = beatmap.CurrentDir;

	for (auto& event : beatmap.Events) {
		switch (event.Type) {
			case Osu::OsuEventType::Background: {
				std::string fileName = event.params[0];
				fileName.erase(std::remove(fileName.begin(), fileName.end(), '\"'), fileName.end());

				m_backgroundFile = fileName;
				break;
			}

			case Osu::OsuEventType::Sample: {
				std::string fileName = event.params[1];
				fileName.erase(std::remove(fileName.begin(), fileName.end(), '\"'), fileName.end());

				auto path = beatmap.CurrentDir / fileName;
				if (std::filesystem::exists(path)) {
					AutoSample sample = {};
					sample.StartTime = event.StartTime;
					sample.Index = beatmap.GetCustomSampleIndex(fileName);
					sample.Volume = 1;
					sample.Pan = 0;

					m_autoSamples.push_back(sample);
				}
				
				break;
			}
		}
	}

	{
		AutoSample sample = {};
		sample.StartTime = beatmap.AudioLeadIn;
		sample.Index = beatmap.GetCustomSampleIndex(beatmap.AudioFilename);
		sample.Volume = 1;
		sample.Pan = 0;

		m_autoSamples.push_back(sample);
	}

	for (auto& note : beatmap.HitObjects) {
		NoteInfo info = {};
		info.StartTime = note.StartTime;
		info.Type = NoteType::NORMAL;
		info.Keysound = note.KeysoundIndex;
		info.LaneIndex = static_cast<int>(std::floorf(note.X * static_cast<float>(beatmap.CircleSize) / 512.0f));
		info.Volume = static_cast<float>(note.Volume) / 100.0;
		info.Pan = 0;

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
			info.Value = 60000.0 / timing.BeatLength;
			info.TimeSignature = timing.TimeSignature;
			info.Type = TimingType::BPM;

			m_bpms.push_back(info);
		}
	}

	auto audioManager = AudioManager::GetInstance();
	for (int i = 0; i < beatmap.HitSamples.size(); i++) {
		auto& keysound = beatmap.HitSamples[i];

		auto path = beatmap.CurrentDir / keysound;

		Sample sm = {};
		sm.FileName = path;
		sm.Index = i;
		
		m_samples.push_back(sm);
	}

	for (auto& note : m_notes) {
		switch (m_keyCount) {
			case 4: {
				if (note.LaneIndex >= 2) {
					note.LaneIndex += 3;
				}
				break;
			}

			case 5: {
				if (note.LaneIndex == 3) {
					note.LaneIndex += 1;
				}
				else if (note.LaneIndex >= 4) {
					note.LaneIndex += 2;
				}
				break;
			}
		}
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

	m_beatmapDirectory = file.CurrentDir;
	m_title = std::u8string(file.Title.begin(), file.Title.end());
	m_audio = "";// file.FileDirectory + "/" + "test.mp3";
	m_keyCount = 7;
	m_artist = std::u8string(file.Artist.begin(), file.Artist.end());
	m_backgroundFile = file.StageFile;
	BaseBPM = file.BPM;
	m_customMeasures = file.Measures;

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
		info.Volume = 1;
		info.Pan = 0;
		
		if (note.EndTime != -1) {
			info.Type = NoteType::HOLD;
			info.EndTime = note.EndTime;
		}

		// check if overlap lastTime
		if (info.StartTime < lastTime[info.LaneIndex]) {
			::printf("[Warning] overlapped note found at %d ms and conflict with %d ms\n", info.StartTime, lastTime[info.LaneIndex]);

			if (note.SampleIndex != -1) {
				AutoSample sm = {};
				sm.StartTime = note.StartTime;
				sm.Index = note.SampleIndex;
				sm.Volume = 1;
				sm.Pan = 0;

				m_autoSamples.push_back(sm);
			}
		}
		else {
			lastTime[info.LaneIndex] = info.Type == NoteType::HOLD ? info.EndTime : info.StartTime;
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
		sm.FileName = file.CurrentDir / sample.second;
		sm.Index = sample.first;

		m_samples.push_back(sm);
	}

	for (auto& autoSample : file.AutoSamples) {
		AutoSample sm = {};
		sm.StartTime = autoSample.StartTime;
		sm.Index = autoSample.SampleIndex;
		sm.Volume = 1;
		sm.Pan = 1;

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

	PredefinedAudioLength = file.AudioLength;
	NormalizeTimings();
	ComputeKeyCount();
	ComputeHash();
}

Chart::Chart(O2::OJN& file, int diffIndex) {
	auto& diff = file.Difficulties[diffIndex];

	int len = strlen(file.Header.title);

	auto str = CodepageToUtf8(file.Header.title, len, 949);

	m_title = str;
	m_backgroundBuffer = file.BackgroundImage;
	m_keyCount = 7;
	m_customMeasures = diff.Measures;

	int lastTime[7] = {};
	for (auto& note : diff.Notes) {
		NoteInfo info = {};
		info.StartTime = note.StartTime;
		info.Keysound = note.SampleRefId;
		info.LaneIndex = note.LaneIndex;
		info.Type = NoteType::NORMAL;
		info.Volume = note.Volume;
		info.Pan = note.Pan;

		if (note.IsLN) {
			info.Type = NoteType::HOLD;
			info.EndTime = note.EndTime;
		}

		// check if overlap lastTime
		if (info.StartTime < lastTime[info.LaneIndex]) {
			::printf("[Warning] overlapped note found at %d ms and conflict with %d ms\n", info.StartTime, lastTime[info.LaneIndex]);

			if (note.SampleRefId != -1) {
				AutoSample sm = {};
				sm.StartTime = note.StartTime;
				sm.Index = note.SampleRefId;
				sm.Volume = note.Volume;
				sm.Pan = note.Pan;

				m_autoSamples.push_back(sm);
			}
		}
		else {
			lastTime[info.LaneIndex] = info.Type == NoteType::HOLD ? info.EndTime : info.StartTime;
			m_notes.push_back(info);
		}
	}

	for (auto& timing : diff.Timings) {
		TimingInfo info = {};
		info.StartTime = timing.Time;
		info.Value = timing.BPM;
		info.TimeSignature = 4;
		info.Type = TimingType::BPM;

		m_bpms.push_back(info);
	}

	m_bpms[0].Beat = 0;
	for (int i = 1; i < m_bpms.size(); i++) {
		m_bpms[i].Beat = m_bpms[i - 1].Beat + (m_bpms[i].StartTime - m_bpms[i - 1].StartTime) * m_bpms[i - 1].Value / 60000.0f;
	}

	for (auto& autoSample : diff.AutoSamples) {
		AutoSample sm = {};
		sm.StartTime = autoSample.StartTime;
		sm.Index = autoSample.SampleRefId;
		sm.Volume = autoSample.Volume;
		sm.Pan = autoSample.Pan;

		m_autoSamples.push_back(sm);
	}

	for (auto& sample : diff.Samples) {
		Sample sm = {};
		sm.FileBuffer = sample.AudioData;
		sm.Index = sample.RefValue;
		sm.Type = 2;
		
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

	PredefinedAudioLength = diff.AudioLength;
	NormalizeTimings();
	ComputeKeyCount();
	ComputeHash();
}

Chart::~Chart() {
	
}

int Chart::GetLength() {
	if (PredefinedAudioLength != -1) {
		return PredefinedAudioLength;
	}

	return m_notes[m_notes.size() - 1].EndTime != 0 
		? m_notes[m_notes.size() - 1].EndTime 
		: m_notes[m_notes.size() - 1].StartTime;
}

void Chart::ApplyMod(Mod mod) {
	switch (mod) {
		case Mod::MIRROR: {
			for (auto& note : m_notes) {
				note.LaneIndex = m_keyCount - 1 - note.LaneIndex;
			}
			break;
		}

		case Mod::RANDOM: {
			std::vector<int> lanes(m_keyCount);
			for (int i = 0; i < 7; i++) {
				lanes[i] = i;
			}

			auto rng = std::default_random_engine{};
			std::shuffle(std::begin(lanes), std::end(lanes), rng);

			for (auto& note : m_notes) {
				note.LaneIndex = lanes[note.LaneIndex];
			}
			break;
		}
	}
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

void Chart::ComputeKeyCount() {
	bool Lanes[7] = { false, false, false, false, false, false };

	for (auto& note : m_notes) {
		if (!Lanes[note.LaneIndex]) {
			Lanes[note.LaneIndex] = true;
		}
	}

	// BMS-O2 4K is: X X - - - X X
	// BMS-O2 5K is: X X - X - X X
	// BMS-O2 6K is: X X X X X X -
	// BMS-O2 7K is: X X X X X X X

	// Check for 7K first since it has the highest priority
	if (Lanes[0] && Lanes[1] && Lanes[2] && Lanes[3] && Lanes[4] && Lanes[5] && Lanes[6]) {
		m_keyCount = 7;
	}
	// Check for 6K
	else if (Lanes[0] && Lanes[1] && Lanes[2] && Lanes[3] && Lanes[4] && Lanes[5] && !Lanes[6]) {
		m_keyCount = 6;
	}
	// Check for 5K
	else if (Lanes[0] && Lanes[1] && !Lanes[2] && Lanes[3] && !Lanes[4] && Lanes[5] && Lanes[6]) {
		m_keyCount = 5;
	}
	// Check for 4K
	else if (Lanes[0] && Lanes[1] && !Lanes[2] && !Lanes[3] && !Lanes[4] && Lanes[5] && Lanes[6]) {
		m_keyCount = 4;
	}
	// Otherwise, the pattern does not match any of the known K values
	else {
		std::cout << "Unknown lane pattern, fallback to 7K" << std::endl;
		m_keyCount = 7;
	}
}
