/*
 * MIT License
 *
 * Copyright (c) 2023 Estrol Mendex
 * See the LICENSE file in the root of this project for details.
 */

#ifndef __AUDIOSAMPLE_H_
#define __AUDIOSAMPLE_H_

#include <EstAudio/include/EstAudio.h>
#include <EstAudio/include/EstEncoder.h>
#include <filesystem>
#include <memory>
#include <vector>

namespace Audio {
    class Sample
    {
    public:
        Sample();
        ~Sample();

        void Load(std::filesystem::path path);
        void Load(const char *buf, size_t size);
        void Load(ECHANDLE encoder);

        void Play();
        void Stop();

        void SetVolume(float volume);
        void SetRate(float pitch);
        void SetPan(float pan);
        void SetPitch(bool enable);

        float GetVolume() const;

    private:
        void Preload();
        void Destroy();

        std::vector<char> m_Buffer;

        uint64_t m_Id;
        EHANDLE  m_Handle;

        float m_Volume;
        float m_Rate;
        float m_Pan;
        bool  m_Pitch;
    };
} // namespace Audio

#endif