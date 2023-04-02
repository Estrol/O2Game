#include "GameAudioSampleCache.hpp"
#include <iostream>
#include <filesystem>
#include <vector>
#include "../Data/Chart.hpp"
#include "../../Engine/EstEngine.hpp"

struct NoteAudioSample {
	std::string FilePath;
	AudioSample* Sample;
};

namespace GameAudioSampleCache {
	std::unordered_map<int, NoteAudioSample> samples;
	std::unordered_map<int, AudioSampleChannel*> sampleIndex;
}

int LastIndexOf(std::string& str, char c) {
	for (int i = str.length() - 1; i >= 0; i--) {
		if (str[i] == c) {
			return i;
		}
	}

	return -1;
}

void GameAudioSampleCache::Load(Chart* chart) {
	auto audioManager = AudioManager::GetInstance();
	Dispose();

	std::vector<std::string> ext = { ".wav", ".ogg", ".mp3" };

	for (auto& it : chart->m_samples) {
		NoteAudioSample sample = {};
		std::string path = it.FileName;

		// check if given path has ext
		bool alreadyHasExt = false;
		int lastIndex = LastIndexOf(path, '.');
		if (lastIndex > 0) {
			path = it.FileName.substr(0, lastIndex);
			
			std::string extensions = it.FileName.substr(lastIndex);
			ext.push_back(extensions);
		}

		bool found = false;
		for (auto& fileExt : ext) {
			std::string tmpPath = path + fileExt;
			if (std::filesystem::exists(tmpPath)) {
				path = tmpPath;
				found = true;
				break;
			}
		}

		if (!found) {
			found = std::filesystem::exists(it.FileName);

			if (found) {
				path = it.FileName;
			}
		}

		if (found) {
			sample.FilePath = path;

			if (audioManager->GetSample(path + std::to_string(it.Index)) == nullptr) {
				if (!audioManager->CreateSample(path + std::to_string(it.Index), path, &sample.Sample)) {
					std::cout << "Failed to load sample: " << it.FileName << std::endl;
					continue;
				}

				::printf("Loading audio: %s, at index: %d\n", path.c_str(), it.Index);
				samples[it.Index] = sample;
			}
		}
		else {
			sample.FilePath = path;
			::printf("Cannot find audio: %s, at index: %d, Creating a silent audio\n", path.c_str(), it.Index);

			if (audioManager->GetSample(path + std::to_string(it.Index)) == nullptr) {
				if (!audioManager->CreateSample(path + std::to_string(it.Index), "", &sample.Sample)) {
					std::cout << "Failed to load sample: " << it.FileName << std::endl;
					continue;
				}

				samples[it.Index] = sample;
			}
		}
	}

	/* Pre-load the BASS Audio */
	if (samples.size()) {
		auto ch = samples[0].Sample->CreateChannel();
		ch->SetVolume(0);
		ch->Play();
	}
}

void GameAudioSampleCache::Play(int index, int volume) {
	if (samples.find(index) == samples.end()) {
		return;
	}

	::printf("Playing index at: %d\n", index);

	auto& sample = samples[index];
	auto channel = sample.Sample->CreateChannel().release();

	if (sampleIndex.find(index) != sampleIndex.end()) {
		auto ch = sampleIndex[index];
		if (ch->IsPlaying()) {
			ch->Stop();
		}
	}
	
	sampleIndex[index] = channel;
	channel->SetVolume(volume);
	channel->Play();
}

void GameAudioSampleCache::Stop(int index) {
	if (sampleIndex.find(index) != sampleIndex.end()) {
		auto& it = sampleIndex[index];

		if (it->IsPlaying()) {
			it->Stop();
		}

		sampleIndex.erase(index);
	}
}

void GameAudioSampleCache::ResumeAll() {
	for (auto& kv : sampleIndex) {
		kv.second->Play();
	}
}

void GameAudioSampleCache::PauseAll() {
	for (auto& kv : sampleIndex) {
		if (kv.second->IsPlaying()) {
			kv.second->Pause();
		}
		else {
			sampleIndex.erase(kv.first);
		}
	}
}

void GameAudioSampleCache::StopAll() {
	for (auto& kv : sampleIndex) {
		if (kv.second->IsPlaying()) {
			kv.second->Stop();
		}

		sampleIndex.erase(kv.first);
	}
}

void GameAudioSampleCache::Dispose() {
	for (auto& kv : sampleIndex) {
		delete kv.second;
	}

	auto audioManager = AudioManager::GetInstance();
	for (auto& it : samples) {
		audioManager->RemoveSample(it.second.FilePath + std::to_string(it.first));
	}

	sampleIndex.clear();
	samples.clear();
}
