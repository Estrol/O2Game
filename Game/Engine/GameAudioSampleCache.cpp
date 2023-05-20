#include "GameAudioSampleCache.hpp"
#include <iostream>
#include <filesystem>
#include <vector>
#include <fstream>

#include "../Data/Chart.hpp"
#include "../../Engine/EstEngine.hpp"
#include "../../Engine/BassFXSampleEncoding.hpp"

struct NoteAudioSample {
	std::string FilePath;
	AudioSample* Sample;
};

namespace GameAudioSampleCache {
	std::unordered_map<int, NoteAudioSample> samples;
	std::unordered_map<int, AudioSampleChannel*> sampleIndex;

	std::string currentHash;
	double m_rate = 1.0;
}

int LastIndexOf(std::string& str, char c) {
	for (int i = str.length() - 1; i >= 0; i--) {
		if (str[i] == c) {
			return i;
		}
	}

	return -1;
}

void GameAudioSampleCache::Load(Chart* chart, bool pitch) {
	auto audioManager = AudioManager::GetInstance();
	if (currentHash == chart->MD5Hash) {
		return;
	}

	Dispose();
	currentHash = chart->MD5Hash;

	std::vector<std::string> ext = { ".wav", ".ogg", ".mp3" };

	for (auto& it : chart->m_samples) {
		NoteAudioSample sample = {};

		if (it.Type == 2) {
			sample.FilePath = "Internal" + std::to_string(it.Index);
			
			if (audioManager->GetSample(sample.FilePath) == nullptr) {
				if (!pitch && m_rate != 1.0f) {
					auto data = BASS_FX_SampleEncoding::Encode(it.FileBuffer.data(), it.FileBuffer.size(), m_rate);
					if (std::get<0>(data) == 0) {
						std::cout << "Failed to preprocess audio tempo for non-pitch sample: " << it.FileName << std::endl;
						continue;
					}

					if (!audioManager->CreateSampleFromData(
						sample.FilePath, 
						std::get<0>(data), 
						std::get<1>(data), 
						std::get<2>(data),
						std::get<3>(data), 
						std::get<4>(data),
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

					char* buffer = new char[size];
					fs.read(buffer, size);
					fs.close();

					auto data = BASS_FX_SampleEncoding::Encode(buffer, size, m_rate);
					delete[] buffer;

					if (std::get<0>(data) == 0) {
						std::cout << "Failed to preprocess audio tempo for non-pitch sample: " << it.FileName << std::endl;
						continue;
					}

					if (!audioManager->CreateSampleFromData(
						sample.FilePath + std::to_string(it.Index),
						std::get<0>(data),
						std::get<1>(data),
						std::get<2>(data),
						std::get<3>(data),
						std::get<4>(data),
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

void GameAudioSampleCache::Play(int index, int volume) {
	if (index == -1) {
		return;
	}

	if (samples.find(index) == samples.end()) {
		return;
	}

	Stop(index);
	
	auto channel = samples[index].Sample->CreateChannel().release();
	
	sampleIndex[index] = channel;
	channel->SetVolume(volume);
	bool result = channel->Play();
	if (!result) {
		::printf("Failed to play index %d\n", index);
	}
}

void GameAudioSampleCache::Stop(int index) {
	if (sampleIndex.find(index) != sampleIndex.end()) {
		auto& it = sampleIndex[index];

		if (it->IsPlaying()) {
			it->Stop();
		}

		delete it;
		it = nullptr;
		
		sampleIndex.erase(index);
	}
}

void GameAudioSampleCache::SetRate(double rate) {
	m_rate = rate;
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
		if (kv.second != nullptr) {
			if (kv.second->IsPlaying()) {
				kv.second->Stop();
			}

			delete kv.second;
			kv.second = nullptr;
		}
	}

	sampleIndex.clear();
}

void GameAudioSampleCache::Dispose() {
	StopAll();

	samples.clear();
	AudioManager::GetInstance()->RemoveAll();
}
