#include "BGMPreview.hpp"
#include "GameAudioSampleCache.hpp"
#include "../Data/Chart.hpp"
#include "../Resources/Configuration.hpp"
#include "../EnvironmentSetup.hpp"
#include "../Data/MusicDatabase.h"

BGMPreview::~BGMPreview() {
	
}

void BGMPreview::Load(int index) {
	OnStarted = false;
	OnPause = false;
	m_rate = 1;
	m_bgmIndex = index;

	std::thread([&] {
		DB_MusicItem* item = MusicDatabase::GetInstance()->GetArrayItem() + m_bgmIndex;

		auto path = Configuration::Load("Music", "Folder");
		std::filesystem::path file = path;
		file /= "o2ma" + std::to_string(item->Id) + ".ojn";

		try {
			O2::OJN o2jamFile;
			o2jamFile.Load(file);

			if (!o2jamFile.IsValid()) {
				return;
			}

			if (m_currentChart) {
				delete m_currentChart;
			}

			m_currentChart = new Chart(o2jamFile, 2);
		}
		catch (std::exception) {
			std::cout << "[BGMPreview] Failed to load the audio chart!" << std::endl;
			return;
		}

		m_autoSamples.clear();

		for (auto& sample : m_currentChart->m_autoSamples) {
			m_autoSamples.push_back(sample);
		}

		for (auto& note : m_currentChart->m_notes) {
			if (note.Keysound != -1) {
				AutoSample sm = {};
				sm.StartTime = note.StartTime;
				sm.Index = note.Keysound;
				sm.Volume = note.Volume;
				sm.Pan = note.Pan;

				m_autoSamples.push_back(sm);
			}
		}

		std::sort(m_autoSamples.begin(), m_autoSamples.end(), [](const AutoSample& a, const AutoSample& b) {
			return a.StartTime < b.StartTime;
		});

		m_currentAudioPosition = std::clamp(static_cast<int>(m_autoSamples.front().StartTime) - 200, 0, INT_MAX);
		m_currentSampleIndex = 0;
		m_length = m_currentChart->GetLength();

		GameAudioSampleCache::SetRate(1.0);
		GameAudioSampleCache::Load(m_currentChart, Configuration::Load("Game", "AudioPitch") == "1");

		if (m_callback) {
			m_callback(true);
		}
	}).detach();
}

void BGMPreview::Update(double delta) {
	if (OnPause || !OnStarted) return;

	m_currentAudioPosition += (delta * m_rate) * 1000;

	for (int i = m_currentSampleIndex; i < m_autoSamples.size(); i++) {
		auto& sample = m_autoSamples[i];

		if (m_currentAudioPosition >= sample.StartTime) {
			GameAudioSampleCache::Play(sample.Index, sample.Volume * 100, sample.Pan * 100);
			m_currentSampleIndex++;
		}
		else {
			break;
		}
	}

	if (m_currentAudioPosition > m_length + 200) {
		m_callback(false);
		OnStarted = false;
	}
}

void BGMPreview::Play() {
	OnStarted = true;
}

void BGMPreview::Stop() {
	OnStarted = false;

	m_callback(false);
	GameAudioSampleCache::StopAll();
}

void BGMPreview::Reload() {
	OnPause = true;

	GameAudioSampleCache::Load(m_currentChart, Configuration::Load("Game", "AudioPitch") == "1", true);

	OnPause = false;
}

void BGMPreview::OnReady(std::function<void(bool)> callback) {
	m_callback = callback;
}