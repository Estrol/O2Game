/*
 * MIT License
 *
 * Copyright (c) 2023 Estrol Mendex
 * See the LICENSE file in the root of this project for details.
 */

#ifndef __AUDIOENCODER_H_
#define __AUDIOENCODER_H_

#include <EstAudio/include/EstEncoder.h>

#include "AudioSample.h"
#include "AudioStream.h"

#include <filesystem>

namespace Audio {

    class Encoder
    {
    public:
        Encoder();
        ~Encoder();

        void Load(std::filesystem::path path);
        void Load(const char *buf, size_t size);
        void Load(ECHANDLE encoder);

        void SetTempo(float tempo);
        void SetPitch(float pitch);
        void SetSampleRate(float rate);

        void SetPan(float pan);
        void SetVolume(float volume);

        float GetTempo() const;
        float GetPitch() const;
        float GetSampleRate() const;

        float GetPan() const;
        float GetVolume() const;

        void Render();
        void Destroy();

        Audio::Sample *GetSample();
        Audio::Stream *GetStream();

    private:
        ECHANDLE m_Handle;
    };
} // namespace Audio

#endif