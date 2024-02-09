/*
 * MIT License
 *
 * Copyright (c) 2023 Estrol Mendex
 * See the LICENSE file in the root of this project for details.
 */

#ifndef __AUDIOENGINE_H_
#define __AUDIOENGINE_H_

#include <EstAudio/include/EstAudio.h>
#include <EstAudio/include/EstEncoder.h>
#include <filesystem>
#include <memory>
#include <unordered_map>
#include <vector>

namespace Audio {
    class Sample;
    class Stream;

    struct SampleDecoder
    {
        uint64_t Id;
        EHANDLE  Handle;
    };

    class Engine
    {
    public:
        void Init();

        Stream *LoadStream(std::filesystem::path path);
        Stream *LoadStream(const char *buf, size_t size);
        Stream *LoadStream(ECHANDLE encoder);

        Sample *LoadSample(std::filesystem::path path);
        Sample *LoadSample(const char *buf, size_t size);
        Sample *LoadSample(ECHANDLE encoder);

        void Destroy(Sample *sample);
        void Destroy(Stream *stream);

        void DestroyAll();

        static Engine *Get();
        static void    Destroy();

    private:
        Engine();
        ~Engine();

        std::vector<std::unique_ptr<Sample>> m_Samples;
        std::vector<std::unique_ptr<Stream>> m_Streams;

        std::vector<std::unique_ptr<Stream>> m_StreamsToDestroy;

        bool m_initialized;
    };
} // namespace Audio

#endif