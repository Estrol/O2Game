#include "GameAudioSampleCache.hpp"
#include <iostream>
#include <filesystem>
#include <vector>
#include <array>
#include <fstream>
#include <mutex>

#include "../Data/Chart.hpp"
#include "Audio/AudioManager.h"
#include "Audio/BassFXSampleEncoding.h"

struct NoteAudioSample {
	std::string FilePath;
	AudioSample* Sample;
};

namespace GameAudioSampleCache {
	std::unordered_map<int, NoteAudioSample> samples;
	std::unordered_map<int, std::unique_ptr<AudioSampleChannel>> sampleIndex;

	std::string currentHash;
	double m_rate = 1.0;

	std::mutex m_lock;
}

int LastIndexOf(std::string& str, char c) {
	for (int i = (int)str.length() - 1; i >= 0; i--) {
		if (str[i] == c) {
			return i;
		}
	}

	return -1;
}

void GameAudioSampleCache::Load(Chart* chart, bool pitch, bool force) {
	currentHash = "";

	Load(chart, pitch);
}

bool GameAudioSampleCache::IsEmpty() {
	return currentHash == "";
}

void GameAudioSampleCache::Load(Chart* chart, bool pitch) {
	auto audioManager = AudioManager::GetInstance();
	if (currentHash == chart->MD5Hash) {
		return;
	}

	Dispose();
	currentHash = chart->MD5Hash;

	std::array<std::string, 3> ext = { ".wav", ".ogg", ".mp3" };

	for (auto& it : chart->m_samples) {
		NoteAudioSample sample = {};

		if (it.Type == 2) {
			sample.FilePath = "Internal" + std::to_string(it.Index);
			
			if (audioManager->GetSample(sample.FilePath) == nullptr) {
				if (!pitch && m_rate != 1.0f) {
					auto data = BASS_FX_SampleEncoding::Encode(it.FileBuffer.data(), it.FileBuffer.size(), (float)m_rate);
					if (data.sampleFlags == 0) {
						std::cout << "Failed to preprocess audio tempo for non-pitch sample: " << it.FileName << std::endl;
						continue;
					}

					if (!audioManager->CreateSampleFromData(
						sample.FilePath,
						data.sampleFlags,
						data.sampleRate,
						data.sampleChannels,
						data.sampleLength,
						data.sampleData.data(),
						&sample.Sample)) {
						
						std::cout << "Failed to load sample: " << it.FileName << std::endl;
						continue;
					}
				}
				else {
					if (!audioManager->CreateSample(sample.FilePath, it.FileBuffer.data(), it.FileBuffer.size(), &sample.Sample)) {
						std::cout << "Failed to load sample: " << it.FileName << std::endl;
						continue;
					}

					sample.Sample->SetRate(m_rate);
				}

				//::printf("Loading audio: %s, at index: %d\n", path.c_str(), it.Index);
				samples[it.Index] = sample;
			}
		}
		else {
			auto File = it.FileName;
			std::filesystem::path path;

			bool found = false;
			for (auto& fileExt : ext) {
				auto tmpPath = File.replace_extension(fileExt);
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
				sample.FilePath = path.string();

				if (!pitch && m_rate != 1.0f) {
					std::fstream fs(path, std::ios::binary | std::ios::in);
					if (!fs.is_open()) {
						std::cout << "Failed to open file: " << path.string() << std::endl;
						continue;
					}

					fs.seekg(0, std::ios::end);
					size_t size = fs.tellg();
					fs.seekg(0, std::ios::beg);

					std::vector<char> buffer(size);
					fs.read(buffer.data(), size);
					fs.close();

					auto data = BASS_FX_SampleEncoding::Encode(buffer.data(), size, (float)m_rate);

					if (data.sampleFlags == 0) {
						std::cout << "Failed to preprocess audio tempo for non-pitch sample: " << it.FileName << std::endl;
						continue;
					}

					if (!audioManager->CreateSampleFromData(
						sample.FilePath + std::to_string(it.Index),
						data.sampleFlags,
						data.sampleRate,
						data.sampleChannels,
						data.sampleLength,
						data.sampleData.data(),
						&sample.Sample)) {

						std::cout << "Failed to load sample: " << it.FileName << std::endl;
						continue;
					}

					samples[it.Index] = sample;
				}
				else {
					if (audioManager->GetSample(path.string() + std::to_string(it.Index)) == nullptr) {
						if (!audioManager->CreateSample(path.string() + std::to_string(it.Index), path, &sample.Sample)) {
							std::cout << "Failed to load sample: " << it.FileName << std::endl;
							continue;
						}

						sample.Sample->SetRate(m_rate);
						samples[it.Index] = sample;
					}
				}
			}
			else {
				sample.FilePath = path.string();
				::printf("Cannot find audio: %s, at index: %d, Creating a silent audio\n", path.string().c_str(), it.Index);

				if (audioManager->GetSample(path.string() + std::to_string(it.Index)) == nullptr) {
					if (!audioManager->CreateSample(path.string() + std::to_string(it.Index), "", &sample.Sample)) {
						std::cout << "Failed to load sample: " << it.FileName << std::endl;
						continue;
					}

					samples[it.Index] = sample;
				}
			}
		}
	}
}

void GameAudioSampleCache::Play(int index, int volume, int pan) {
	if (index == -1) {
		return;
	}

	if (samples.find(index) == samples.end()) {
		return;
	}

	Stop(index);
	
	auto channel = samples[index].Sample->CreateChannel();
	channel->SetVolume(volume);
	channel->SetPan(pan);

	bool result = channel->Play();
	
	sampleIndex[index] = std::move(channel);
	if (!result) {
		::printf("Failed to play index %d\n", index);
	}
}

void GameAudioSampleCache::Stop(int index) {
	std::lock_guard<std::mutex> lock(m_lock);

	if (sampleIndex.find(index) != sampleIndex.end()) {
		auto& it = sampleIndex[index];

		if (it->IsPlaying()) {
			it->Stop();
		}

		sampleIndex.erase(index);
	}
}

void GameAudioSampleCache::SetRate(double rate) {
	if (m_rate != rate) {
		currentHash = "";
	}

	m_rate = rate;
}

double GameAudioSampleCache::SetRate() {
	return m_rate;
}

void GameAudioSampleCache::ResumeAll() {
	std::lock_guard<std::mutex> lock(m_lock);

	for (auto& kv : sampleIndex) {
		kv.second->Play();
	}
}

void GameAudioSampleCache::PauseAll() {
	std::lock_guard<std::mutex> lock(m_lock);

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
	std::lock_guard<std::mutex> lock(m_lock);

	for (auto& kv : sampleIndex) {
		if (kv.second != nullptr) {
			if (kv.second->IsPlaying()) {
				kv.second->Stop();
			}
		}
	}

	sampleIndex.clear();
}

void GameAudioSampleCache::Dispose() {
	StopAll();

	samples.clear();
	AudioManager::GetInstance()->RemoveAll();

	currentHash = "";
}
