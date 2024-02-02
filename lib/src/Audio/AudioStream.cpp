/*
 * MIT License
 *
 * Copyright (c) 2023 Estrol Mendex
 * See the LICENSE file in the root of this project for details.
 */

#include <Audio/AudioEngine.h>
#include <Audio/AudioStream.h>
#include <Exceptions/EstException.h>
#include <Exceptions/EstNotImplemented.h>
#include <fstream>

#include <SDL2/SDL.h>

using namespace Audio;

Stream::Stream()
{
    m_Handle = INVALID_HANDLE;
}

Stream::~Stream()
{
}

void Stream::Load(std::filesystem::path path)
{
    if (!std::filesystem::exists(path)) {
        throw Exceptions::EstException("Failed to load audio stream (path does not exist)");
    }

    auto result = EST_SampleLoad(path.string().c_str(), &m_Handle);
    if (result != EST_OK) {
        throw Exceptions::EstException("Failed to load audio stream (result != EST_OK)");
    }
}

void Stream::Load(const char *buf, size_t size)
{
    if (!buf) {
        throw Exceptions::EstException("Failed to load audio stream (buf == nullptr)");
    }

    auto result = EST_SampleLoadMemory(buf, (int)size, &m_Handle);
    if (result != EST_OK) {
        throw Exceptions::EstException("Failed to load audio stream (result != EST_OK)");
    }
}

void Stream::Load(ECHANDLE encoder)
{
    if (encoder == INVALID_ECHANDLE) {
        throw Exceptions::EstException("Failed to load audio stream (encoder == INVALID_ECHANDLE)");
    }

    auto result = EST_EncoderGetSample(encoder, &m_Handle);
    if (result != EST_OK) {
        throw Exceptions::EstException("Failed to load audio stream (result != EST_OK)");
    }
}

void Stream::Play()
{
    if (!m_Handle) {
        throw Exceptions::EstException("Failed to play audio stream (m_Handle == nullptr)");
    }

    EST_STATUS IsPlaying;
    auto       result = EST_SampleGetStatus(m_Handle, &IsPlaying);

    if (result != EST_OK) {
        throw Exceptions::EstException("Failed to play audio stream (result != EST_OK)");
    }

    if (IsPlaying == EST_STATUS_PLAYING) {
        return;
    }

    result = EST_SamplePlay(m_Handle);
    if (result != EST_OK) {
        throw Exceptions::EstException("Failed to play audio stream (result != EST_OK)");
    }
}

void Stream::Stop()
{
    if (!m_Handle) {
        throw Exceptions::EstException("Failed to stop audio stream (m_Handle == nullptr)");
    }

    EST_STATUS IsPlaying;
    auto       result = EST_SampleGetStatus(m_Handle, &IsPlaying);

    if (result != EST_OK) {
        throw Exceptions::EstException("Failed to stop audio stream (result != EST_OK)");
    }

    if (IsPlaying != EST_STATUS_PLAYING) {
        return;
    }

    result = EST_SampleStop(m_Handle);
    if (result != EST_OK) {
        throw Exceptions::EstException("Failed to stop audio stream (result != EST_OK)");
    }
}

void Stream::Pause()
{
    if (!m_Handle) {
        throw Exceptions::EstException("Failed to pause audio stream (m_Handle == nullptr)");
    }

    throw Exceptions::EstNotImplemented("Stream::Pause()");
}

void Stream::Seek(float miliseconds)
{
}

void Stream::SetVolume(float volume)
{
}

float Stream::GetVolume() const
{
    return 0;
}

float Stream::GetLength() const
{
    return false;
}

float Stream::GetCurrent() const
{
    return 0;
}

void Stream::SetPitch(bool enable)
{
}

void Stream::SetRate(float pitch)
{
}

void Stream::SetPan(float pan)
{
}

bool Stream::IsDone() const
{
    return false;
}
