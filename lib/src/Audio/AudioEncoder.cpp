/*
 * MIT License
 *
 * Copyright (c) 2023 Estrol Mendex
 * See the LICENSE file in the root of this project for details.
 */

#include <Audio/AudioEncoder.h>
#include <Audio/AudioEngine.h>
#include <Exceptions/EstException.h>

using namespace Audio;

Encoder::Encoder()
{
    m_Handle = INVALID_ECHANDLE;
}

Encoder::~Encoder()
{
    Destroy();
}

void Encoder::Load(std::filesystem::path path)
{
    if (!std::filesystem::exists(path)) {
        throw Exceptions::EstException("Failed to load audio encoder");
    }

    auto result = EST_EncoderLoad(path.string().c_str(), NULL, EST_DECODER_UNKNOWN, &m_Handle);
    if (result != EST_OK) {
        throw Exceptions::EstException("Failed to load audio encoder: %s", EST_GetError());
    }
}

void Encoder::Load(const char *buf, size_t size)
{
    if (!buf) {
        throw Exceptions::EstException("Failed to load audio encoder");
    }

    auto result = EST_EncoderLoadMemory(buf, (int)size, NULL, EST_DECODER_UNKNOWN, &m_Handle);
    if (result != EST_OK) {
        throw Exceptions::EstException("Failed to load audio encoder: %s", EST_GetError());
    }
}

void Encoder::Load(ECHANDLE encoder)
{
    if (encoder == INVALID_ECHANDLE) {
        throw Exceptions::EstException("Failed to load audio encoder");
    }

    m_Handle = encoder;
}

void Encoder::SetTempo(float tempo)
{
    EST_RESULT result = EST_EncoderSetAttribute(m_Handle, EST_ATTRIB_ENCODER_TEMPO, tempo);
    if (result != EST_OK) {
        throw Exceptions::EstException("Failed to set audio encoder tempo: %s", EST_GetError());
    }
}

void Encoder::SetPitch(float pitch)
{
    EST_RESULT result = EST_EncoderSetAttribute(m_Handle, EST_ATTRIB_ENCODER_PITCH, pitch);
    if (result != EST_OK) {
        throw Exceptions::EstException("Failed to set audio encoder pitch: %s", EST_GetError());
    }
}

void Encoder::SetSampleRate(float rate)
{
    EST_RESULT result = EST_EncoderSetAttribute(m_Handle, EST_ATTRIB_ENCODER_SAMPLERATE, rate);
    if (result != EST_OK) {
        throw Exceptions::EstException("Failed to set audio encoder rate: %s", EST_GetError());
    }
}

void Encoder::SetVolume(float volume)
{
    EST_RESULT result = EST_EncoderSetAttribute(m_Handle, EST_ATTRIB_VOLUME, volume);
    if (result != EST_OK) {
        throw Exceptions::EstException("Failed to set audio encoder volume: %s", EST_GetError());
    }
}

void Encoder::SetPan(float pan)
{
    EST_RESULT result = EST_EncoderSetAttribute(m_Handle, EST_ATTRIB_PAN, pan);
    if (result != EST_OK) {
        throw Exceptions::EstException("Failed to set audio encoder pan: %s", EST_GetError());
    }
}

float Encoder::GetTempo() const
{
    float tempo = 0.0f;

    EST_RESULT result = EST_EncoderGetAttribute(m_Handle, EST_ATTRIB_ENCODER_TEMPO, &tempo);
    if (result != EST_OK) {
        throw Exceptions::EstException("Failed to get audio encoder tempo: %s", EST_GetError());
    }

    return tempo;
}

float Encoder::GetPitch() const
{
    float pitch = 0.0f;

    EST_RESULT result = EST_EncoderGetAttribute(m_Handle, EST_ATTRIB_ENCODER_PITCH, &pitch);
    if (result != EST_OK) {
        throw Exceptions::EstException("Failed to get audio encoder pitch: %s", EST_GetError());
    }

    return pitch;
}

float Encoder::GetSampleRate() const
{
    float rate = 0.0f;

    EST_RESULT result = EST_EncoderGetAttribute(m_Handle, EST_ATTRIB_ENCODER_SAMPLERATE, &rate);
    if (result != EST_OK) {
        throw Exceptions::EstException("Failed to get audio encoder rate: %s", EST_GetError());
    }

    return rate;
}

float Encoder::GetVolume() const
{
    float volume = 0.0f;

    EST_RESULT result = EST_EncoderGetAttribute(m_Handle, EST_ATTRIB_VOLUME, &volume);
    if (result != EST_OK) {
        throw Exceptions::EstException("Failed to get audio encoder volume: %s", EST_GetError());
    }

    return volume;
}

float Encoder::GetPan() const
{
    float pan = 0.0f;

    EST_RESULT result = EST_EncoderGetAttribute(m_Handle, EST_ATTRIB_PAN, &pan);
    if (result != EST_OK) {
        throw Exceptions::EstException("Failed to get audio encoder pan: %s", EST_GetError());
    }

    return pan;
}

void Encoder::Render()
{
    EST_RESULT result = EST_EncoderRender(m_Handle);
    if (result != EST_OK) {
        throw Exceptions::EstException("Failed to render audio encoder: %s", EST_GetError());
    }
}

void Encoder::Destroy()
{
    if (m_Handle != INVALID_ECHANDLE) {
        EST_EncoderFree(m_Handle);
        m_Handle = INVALID_ECHANDLE;
    }
}

Stream *Encoder::GetStream()
{
    auto engine = Engine::Get();

    return engine->LoadStream(m_Handle);
}

Sample *Encoder::GetSample()
{
    auto engine = Engine::Get();

    return engine->LoadSample(m_Handle);
}