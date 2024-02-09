/*
 * MIT License
 *
 * Copyright (c) 2023 Estrol Mendex
 * See the LICENSE file in the root of this project for details.
 */

#pragma once
#include <functional>
#include <vector>

#include "../../Data/chart.hpp"
#include <Audio/AudioEngine.h>
#include <Audio/AudioSample.h>
#include <Audio/AudioSampleChannel.h>

struct NoteAudioSample
{
    std::string    FilePath;
    Audio::Sample *Sample;
};

namespace SampleManager {
    void Load(Chart *chart, bool pitch);
    void Load(Chart *chart, bool pitch, bool force);

    void OnLoad(std::function<void(int, int)> callback);

    void   Play(int index, int volume = 100, int pan = 0);
    void   Stop(int index);
    void   SetRate(double rate);
    double GetRate();
    void   StopAll();

    std::unordered_map<int, NoteAudioSample> &GetSamples();

    void Dispose();
} // namespace SampleManager