/*
 * MIT License
 *
 * Copyright (c) 2023 Estrol Mendex
 * See the LICENSE file in the root of this project for details.
 */

#ifndef __AUDIOTRACK_H_
#define __AUDIOTRACK_H_

#include <EstAudio/include/EstAudio.h>
#include <filesystem>
#include <memory>
#include <vector>

namespace Audio {
    class Stream
    {
    public:
        Stream();
        virtual ~Stream();

        void Play();
        void Stop();
        void Pause();
        void Seek(float seconds);

        void SetVolume(float volume);
        void SetRate(float pitch);
        void SetPitch(bool enable);
        void SetPan(float pan);

        float GetVolume() const;
        float GetLength() const;
        float GetCurrent() const;

        bool IsDone() const;

        void Load(std::filesystem::path path);
        void Load(const char *buf, size_t size);
        void Load(ECHANDLE encoder);

    private:
        EHANDLE m_Handle;
    };
} // namespace Audio

#endif