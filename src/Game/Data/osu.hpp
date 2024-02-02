/*
 * MIT License
 *
 * Copyright (c) 2023 Estrol Mendex
 * See the LICENSE file in the root of this project for details.
 */

#ifndef __OSU_FORMAT_HEADER
#define __OSU_FORMAT_HEADER

#include <filesystem>
#include <iostream>
#include <vector>

namespace Osu {
    struct OsuTimingPoint
    {
        float   Offset;
        float   BeatLength;
        float   TimeSignature;
        int32_t SampleSet;
        int32_t SampleIndex;
        int32_t Volume;
        bool    Inherited;
        bool    KiaiMode;
    };

    struct OsuHitObject
    {
        float       X;
        float       Y;
        int32_t     StartTime;
        int32_t     Type;
        int32_t     HitSound;
        int32_t     EndTime;
        int32_t     HitSample;
        std::string Additions;
        int32_t     KeysoundIndex;
        int32_t     Volume;
    };

    enum class OsuEventType : uint8_t {
        Background,
        Videos,
        Break,
        Sample
    };

    struct OsuEvent
    {
        OsuEventType             Type;
        double                   StartTime;
        std::vector<std::string> params;
    };

    enum class OsuHitObjectType : uint8_t {
        Circle = 0,
        Slider = 1,
        Spinner = 3,
        ManiaLN = 7,
    };

    class Beatmap
    {
    public:
        Beatmap(std::filesystem::path &file);

        bool IsValid();

        // [Format]
        std::string           PeppyFormat = ""; // L
        std::filesystem::path CurrentDir = "";

        // [General]
        std::string AudioFilename = "";
        int32_t     AudioLeadIn = 0;
        int32_t     PreviewTime = 0;
        int32_t     Countdown = 0;
        std::string SampleSet = "";
        float       StackLeniency = 0.0f;
        int32_t     Mode = 3;
        bool        LetterboxInBreaks = false;
        bool        WidescreenStoryboard = false;
        bool        SpecialStyle = false;

        // [Metadata]
        std::string Title = "";
        std::string TitleUnicode = "";
        std::string Artist = "";
        std::string ArtistUnicode = "";
        std::string Creator = "";
        std::string Version = "";
        std::string Source = "";
        std::string Tags = "";
        int32_t     BeatmapID = 0;
        int32_t     BeatmapSetID = 0;

        // [Events]
        std::string BackgroundFile = "";

        // [Difficulty]
        float HPDrainRate = 0.0f;
        float CircleSize = 0.0f;
        float OverallDifficulty = 0.0f;
        float ApproachRate = 0.0f;
        float SliderMultiplier = 0.0f;
        float SliderTickRate = 0.0f;

        std::vector<OsuTimingPoint> TimingPoints;
        std::vector<OsuHitObject>   HitObjects;
        std::vector<OsuEvent>       Events;
        std::vector<std::string>    HitSamples;

        int GetCustomSampleIndex(std::string);

    private:
        void ParseString(std::stringstream &ss);

        bool bIsValid;
    };
} // namespace Osu

#endif