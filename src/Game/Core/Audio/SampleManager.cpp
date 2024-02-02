/*
 * MIT License
 *
 * Copyright (c) 2023 Estrol Mendex
 * See the LICENSE file in the root of this project for details.
 */

#include "SampleManager.h"
#include <Exceptions/EstException.h>
#include <Logs.h>

#include <array>
#include <fstream>
#include <unordered_map>

#include <Audio/AudioEncoder.h>
#include <Misc/Filesystem.h>

namespace {
    std::unordered_map<int, NoteAudioSample> samples;
    std::unordered_map<int, bool>            samplePlaying;
    // std::unordered_map<int, std::shared_ptr<Audio::SampleChannel>> sampleIndex;

    std::string currentHash;
    double      m_rate = 1.0;

    std::function<void(int, int)> onLoadCallback;
} // namespace

int last_index_of(std::string &str, char c)
{
    int index = -1;
    for (int i = 0; i < str.size(); i++) {
        if (str[i] == c) {
            index = i;
        }
    }

    return index;
}

template <std::size_t N>
std::tuple<bool, std::filesystem::path> find_file_support_ext(std::filesystem::path path, std::array<std::string, N> &ext)
{
    std::filesystem::path result = "";

    bool found = false;
    for (auto &fileExt : ext) {
        auto tmpPath = path.replace_extension(fileExt);
        if (std::filesystem::exists(tmpPath)) {
            found = true;
            result = tmpPath;
            break;
        }
    }

    if (!found) {
        found = std::filesystem::exists(path);

        if (found) {
            result = path;
        }
    }

    return { found, result };
}

void SampleManager::Load(Chart *chart, bool pitch)
{
    Load(chart, pitch, false);
}

void SampleManager::Load(Chart *chart, bool pitch, bool force)
{
    if (force) {
        currentHash = "";
    }

    if (currentHash.empty()) {
        Dispose();
    }

    auto manager = Audio::Engine::Get();
    currentHash = chart->MD5Hash;

    std::array<std::string, 3> ext = { ".wav", ".ogg", ".mp3" };

    struct tr_result
    {
        bool                  found = false;
        std::filesystem::path path = "";
        int                   index = 0;

        std::shared_ptr<Audio::Encoder> encoder;
        NoteAudioSample                 sample = {};
    };

    auto processSampleBuf = [&](std::shared_ptr<tr_result> &tr,
                                const Sample               &it) -> void {
        tr->encoder = std::make_shared<Audio::Encoder>();
        tr->encoder->Load((const char *)it.FileBuffer.data(), (int)it.FileBuffer.size());

        if (pitch) {
            float sampleRate = tr->encoder->GetSampleRate();
            tr->encoder->SetSampleRate(sampleRate * (float)m_rate);
        } else {
            tr->encoder->SetTempo((float)m_rate);
        }

        tr->encoder->Render();
    };

    auto processSampleFile = [&](std::shared_ptr<tr_result> &tr,
                                 const Sample               &it) -> void {
        tr->encoder = std::make_shared<Audio::Encoder>();

        auto find_result = find_file_support_ext(it.FileName, ext);
        if (std::get<0>(find_result)) {
            auto path = std::get<1>(find_result);

            tr->encoder->Load(path);
            tr->path = path;

            if (pitch) {
                float sampleRate = tr->encoder->GetSampleRate();
                tr->encoder->SetSampleRate(sampleRate * (float)m_rate);
            } else {
                tr->encoder->SetTempo((float)m_rate);
            }

            tr->encoder->Render();
        } else {
            throw Exceptions::EstException("Failed to load audio sample: %s", it.FileName.c_str());
        }
    };

    int currentProgress = 0;
    int maxProgress = (int)chart->m_samples.size();

    if (m_rate != 1.0f) {
        std::vector<std::shared_ptr<tr_result>> tr_results;
        std::vector<std::thread>                tr_threads;

        std::mutex mutexLock;

        for (auto &it : chart->m_samples) {
            tr_threads.push_back(std::thread(
                [&tr_results, &it, &ext, &processSampleBuf, &processSampleFile, &mutexLock, &currentProgress, maxProgress] {
                    std::shared_ptr<tr_result> tr = std::make_shared<tr_result>();

                    if (it.Type == 2) {
                        tr->sample.FilePath = "Internal" + std::to_string(it.Index);

                        try {
                            processSampleBuf(tr, it);
                            tr->found = true;
                            tr->index = it.Index;
                        } catch (Exceptions::EstException &e) {
                            Logs::Puts("[AudioSampleManager] %s", e.what());
                        }
                    } else {
                        auto find_result = find_file_support_ext(it.FileName, ext);
                        if (std::get<0>(find_result)) {
                            auto path = std::get<1>(find_result);

                            tr->sample.FilePath = path.string();

                            try {
                                processSampleFile(tr, it);
                                tr->found = true;
                                tr->index = it.Index;
                            } catch (Exceptions::EstException &e) {
                                Logs::Puts("[AudioSampleManager] %s", e.what());
                            }
                        } else {
                            // TODO: silent sample
                        }
                    }

                    std::lock_guard<std::mutex> lock(mutexLock);

                    onLoadCallback(++currentProgress, maxProgress);
                    tr_results.push_back(tr);
                }));
        }

        for (auto &it : tr_threads) {
            it.join();
        }

        for (auto &it : tr_results) {
            if (it->found) {
                it->sample.Sample = it->encoder->GetSample();

                if (it->encoder) {
                    it->encoder.reset();
                }

                samples[it->index] = it->sample;
            }
        }
    } else {
        for (auto &it : chart->m_samples) {
            NoteAudioSample sample;
            sample.Sample = nullptr;

            if (it.Type == 2) {
                sample.FilePath = "Internal" + std::to_string(it.Index);

                try {
                    sample.Sample = Audio::Engine::Get()->LoadSample((const char *)it.FileBuffer.data(), it.FileBuffer.size());
                } catch (Exceptions::EstException &e) {
                    sample.Sample = nullptr;
                    Logs::Puts("[AudioSampleManager] Failed to load sample: %s", e.what());

                    std::fstream fs("F:\\test.wav", std::ios::out | std::ios::binary);
                    fs.write((const char *)it.FileBuffer.data(), it.FileBuffer.size());
                    fs.close();
                }
            } else {
                auto find_result = find_file_support_ext(it.FileName, ext);
                if (std::get<0>(find_result)) {
                    auto path = std::get<1>(find_result);

                    sample.FilePath = path.string();

                    try {
                        sample.Sample = Audio::Engine::Get()->LoadSample(sample.FilePath);
                    } catch (Exceptions::EstException &) {
                        sample.Sample = nullptr;
                        Logs::Puts("[AudioSampleManager] Failed to load sample: %s", it.FileName.c_str());
                    }
                } else {
                    // TODO: silent sample
                    Logs::Puts("[AudioSampleManager] Failed to load sample: %s", it.FileName.c_str());
                }
            }

            if (sample.Sample != nullptr) {
                samples[it.Index] = sample;
            }

            onLoadCallback(++currentProgress, maxProgress);
        }
    }
}

void SampleManager::OnLoad(std::function<void(int, int)> callback)
{
    onLoadCallback = callback;
}

void SampleManager::Play(int index, int volume, int pan)
{
    if (index == -1) {
        return;
    }

    if (samples.find(index) == samples.end()) {
        return;
    }

    Stop(index);

    auto sample = samples[index].Sample;
    if (!sample) {
        return;
    }

    sample->SetVolume((float)volume);
    sample->SetPan((float)pan);

    sample->Play();
    samplePlaying[index] = true;
}

void SampleManager::Stop(int index)
{
    if (samplePlaying[index]) {
        samples[index].Sample->Stop();
        samplePlaying[index] = false;
    }
}

void SampleManager::SetRate(double rate)
{
    if (m_rate != rate) {
        currentHash = "";
    }

    m_rate = rate;
}

double SampleManager::GetRate()
{
    return m_rate;
}

void SampleManager::ResumeAll()
{
    /*for (auto &kv : sampleIndex) {
        kv.second->Play();
    }*/
}

void SampleManager::PauseAll()
{
    /*for (auto &kv : sampleIndex) {
        sampleIndex.erase(kv.first);
    }*/
}

std::unordered_map<int, NoteAudioSample> &SampleManager::GetSamples()
{
    return samples;
}

void SampleManager::Dispose()
{
    Audio::Engine::Get()->DestroyAll();

    samples.clear();
    // sampleIndex.clear();
}
