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

namespace {
	std::vector<NoteAudioSample> samples;
	std::vector<NoteAudioSample> events;

	std::vector<AudioSampleChannel*> channels;
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
			std::string tmpPath = path + "." + fileExt;
			if (std::filesystem::exists(tmpPath)) {
				path = tmpPath;
				found = true;
				break;
			}
		}

		if (found) {
			sample.FilePath = it.FileName;

			if (!audioManager->CreateSample(path, path, &sample.Sample)) {
				std::cout << "Failed to load auto sample: " << it.FileName << std::endl;
				continue;
			}

			events.push_back(sample);
		}
		else {
			sample.FilePath = it.FileName;

			if (!audioManager->CreateSample(path, "", &sample.Sample)) {
				std::cout << "Failed to load auto sample: " << it.FileName << std::endl;
				continue;
			}

			events.push_back(sample);
		}
	}

	for (auto& it : chart->m_autoSamples) {
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
			std::string tmpPath = path + "." + fileExt;
			if (std::filesystem::exists(tmpPath)) {
				path = tmpPath;
				found = true;
				break;
			}
		}

		if (found) {
			sample.FilePath = it.FileName;

			if (!audioManager->CreateSample(path, path, &sample.Sample)) {
				std::cout << "Failed to load auto sample: " << it.FileName << std::endl;
				continue;
			}

			events.push_back(sample);
		}
		else {
			sample.FilePath = it.FileName;

			if (!audioManager->CreateSample(path, "", &sample.Sample)) {
				std::cout << "Failed to load auto sample: " << it.FileName << std::endl;
				continue;
			}

			events.push_back(sample);
		}
	}
}

AudioSampleChannel* GameAudioSampleCache::Play(int index, int volume) {
	return nullptr;

	auto& sample = samples[index];
	auto channel = sample.Sample->CreateChannel().release();

	channel->SetVolume(volume);
	channel->Play();

	channels.push_back(channel);

	return channel;
}

AudioSampleChannel* GameAudioSampleCache::PlayEvent(int index, int volume) {
	return nullptr;

	auto& sample = events[index];
	auto channel = sample.Sample->CreateChannel().release();

	channel->SetVolume(volume);
	channel->Play();

	channels.push_back(channel);

	return channel;
}

void GameAudioSampleCache::ResumeAll() {
	for (auto& it : channels) {
		it->Play();
	}
}

void GameAudioSampleCache::PauseAll() {
	for (auto it = channels.begin(); it != channels.end();) {
		(*it)->Pause();

		if ((*it)->IsStopped()) {
			delete* it;
			it = channels.erase(it);
		}
		else {
			it++;
		}
	}
}

void GameAudioSampleCache::StopAll() {
	for (auto& it : channels) {
		it->Stop();
	}
}

void GameAudioSampleCache::Dispose() {
	for (auto& it : channels) {
		delete it;
	}

	auto audioManager = AudioManager::GetInstance();
	for (auto& it : samples) {
		audioManager->RemoveSample(it.FilePath);
	}

	for (auto& it : events) {
		audioManager->RemoveSample(it.FilePath);
	}

	channels.clear();
	samples.clear();
	events.clear();
}
