/*
 * MIT License
 *
 * Copyright (c) 2023 Estrol Mendex
 * See the LICENSE file in the root of this project for details.
 */

#include <Audio/AudioEngine.h>
#include <Audio/AudioSample.h>
#include <Audio/AudioStream.h>
#include <Exceptions/EstException.h>
#include <Misc/Filesystem.h>

#include <array>

using namespace Audio;

static Engine *s_Instance = nullptr;

Engine *Engine::Get()
{
    if (s_Instance == nullptr) {
        s_Instance = new Engine;
    }

    return s_Instance;
}

void Engine::Destroy()
{
    if (s_Instance != nullptr) {
        delete s_Instance;
    }
}

Engine::Engine()
{
    m_initialized = false;
}

Engine::~Engine()
{
    if (m_initialized) {
        DestroyAll();

        EST_DeviceFree();
    }
}

void Engine::Init()
{
    auto result = EST_DeviceInit(44100, EST_DEVICE_STEREO);
    if (result != EST_OK) {
        throw Exceptions::EstException("Failed to init Audio Device: %s", EST_GetError());
    }
}

Stream *Engine::LoadStream(std::filesystem::path path)
{
    auto stream = std::make_unique<Stream>();
    stream->Load(path);

    m_Streams.push_back(std::move(stream));
    return m_Streams.back().get();
}

Stream *Engine::LoadStream(const char *buf, size_t size)
{
    auto stream = std::make_unique<Stream>();
    stream->Load(buf, size);

    m_Streams.push_back(std::move(stream));
    return m_Streams.back().get();
}

Stream *Engine::LoadStream(ECHANDLE handle)
{
    auto stream = std::make_unique<Stream>();
    stream->Load(handle);

    m_Streams.push_back(std::move(stream));
    return m_Streams.back().get();
}

Sample *Engine::LoadSample(std::filesystem::path path)
{
    auto sample = std::make_unique<Sample>();
    sample->Load(path);

    m_Samples.push_back(std::move(sample));
    return m_Samples.back().get();
}

Sample *Engine::LoadSample(const char *buf, size_t size)
{
    auto sample = std::make_unique<Sample>();
    sample->Load(buf, size);

    m_Samples.push_back(std::move(sample));
    return m_Samples.back().get();
}

Sample *Engine::LoadSample(ECHANDLE encoder)
{
    auto sample = std::make_unique<Sample>();
    sample->Load(encoder);

    m_Samples.push_back(std::move(sample));
    return m_Samples.back().get();
}

void Engine::Destroy(Sample *sample)
{
    for (auto it = m_Samples.begin(); it != m_Samples.end(); it++) {
        if ((*it).get() == sample) {
            m_Samples.erase(it);
            break;
        }
    }
}

void Engine::Destroy(Stream *stream)
{
    for (auto it = m_Streams.begin(); it != m_Streams.end(); it++) {
        if ((*it).get() == stream) {
            m_Streams.erase(it);
            break;
        }
    }
}

void Engine::DestroyAll()
{
    m_Samples.clear();
    m_Streams.clear();
}