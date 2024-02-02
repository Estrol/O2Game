/*
 * MIT License
 *
 * Copyright (c) 2023 Estrol Mendex
 * See the LICENSE file in the root of this project for details.
 */

#include <Audio/AudioEngine.h>
#include <Audio/AudioSample.h>
#include <Exceptions/EstException.h>
#include <fstream>
using namespace Audio;

static uint64_t g_SampleId = 0;

Sample::Sample()
{
    m_Id = g_SampleId++;
    m_Pitch = false;
    m_Rate = 1.0f;
    m_Volume = 1.0f;
    m_Pan = 0.0f;

    m_Handle = INVALID_HANDLE;
}

Sample::~Sample()
{
    Destroy();
}

void Sample::Load(std::filesystem::path path)
{
    if (!std::filesystem::exists(path)) {
        throw Exceptions::EstException("Failed to load audio sample");
    }

    std::fstream fs(path, std::ios::binary | std::ios::in);

    fs.seekg(0, std::ios::end);
    size_t size = fs.tellg();
    fs.seekg(0, std::ios::beg);

    m_Buffer.resize(size);
    fs.read(m_Buffer.data(), size);

    fs.close();

    Preload();
}

void Sample::Load(const char *buf, size_t size)
{
    if (!buf) {
        throw Exceptions::EstException("Failed to load audio sample");
    }

    m_Buffer.resize(size);
    memcpy(m_Buffer.data(), buf, size);

    Preload();
}

void Sample::Load(ECHANDLE encoder)
{
    if (encoder == INVALID_ECHANDLE) {
        throw Exceptions::EstException("Failed to load audio sample");
    }

    m_Handle = INVALID_HANDLE;

    EST_RESULT result = EST_EncoderGetSample(encoder, &m_Handle);
    if (result != EST_OK) {
        throw Exceptions::EstException("Failed to load audio sample: %s", EST_GetError());
    }
}

void Sample::Play()
{
    EST_SamplePlay(m_Handle);
}

void Sample::Stop()
{
    EST_SampleStop(m_Handle);
}

void Sample::SetVolume(float volume)
{
    m_Volume = volume / 100.0f;

    EST_SampleSetAttribute(m_Handle, EST_ATTRIB_VOLUME, m_Volume);
}

void Sample::SetRate(float pitch)
{
    m_Rate = pitch;

    EST_SampleSetAttribute(m_Handle, EST_ATTRIB_RATE, m_Rate);
}

void Audio::Sample::SetPan(float pan)
{
    m_Pan = pan;

    EST_SampleSetAttribute(m_Handle, EST_ATTRIB_PAN, m_Pan);
}

void Sample::SetPitch(bool enable)
{
    m_Pitch = enable;

    EST_SampleSetAttribute(m_Handle, EST_ATTRIB_PITCH, m_Pitch);
}

float Sample::GetVolume() const
{
    return m_Volume;
}

void Audio::Sample::Preload()
{
    m_Handle = INVALID_HANDLE;

    EST_RESULT result = EST_SampleLoadMemory(
        reinterpret_cast<const void *>(&m_Buffer[0]),
        (int)m_Buffer.size(),
        &m_Handle);

    if (result != EST_OK) {
        throw Exceptions::EstException("Failed to load Audio Sample: %s", EST_GetError());
    }
}

void Audio::Sample::Destroy()
{
    if (m_Handle != INVALID_HANDLE) {
        EST_SampleFree(m_Handle);
        m_Handle = INVALID_HANDLE;
    }
}
