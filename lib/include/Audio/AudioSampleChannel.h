/*
 * MIT License
 *
 * Copyright (c) 2023 Estrol Mendex
 * See the LICENSE file in the root of this project for details.
 */

#ifndef __AUDIOSAMPLECHANNEL_H_
#define __AUDIOSAMPLECHANNEL_H_

namespace Audio {
    class SampleChannel
    {
    public:
        SampleChannel(const char *buffer, size_t size);
        ~SampleChannel();

        void Play();
        void Stop();

        void SetVolume(float volume);
        void SetRate(float pitch);
        void SetPitch(bool enable);
        void SetPan(float pan);

        float GetVolume() const;
        float GetRate() const;
        float GetPan() const;

    private:
        int m_Id;

        bool  pitch;
        float volume;
        float rate;
        float pan;

        bool disposed;
        bool isplayed;
    };
} // namespace Audio

#endif