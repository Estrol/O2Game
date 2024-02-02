/*
 * MIT License
 *
 * Copyright (c) 2023 Estrol Mendex
 * See the LICENSE file in the root of this project for details.
 */

#include "osu.hpp"
#include "Util/Util.hpp"
#include <Logs.h>
#include <filesystem>
#include <fstream>
#include <sstream>

Osu::Beatmap::Beatmap(std::filesystem::path &file)
{
    std::fstream      fs(file, std::ios::in);
    std::stringstream ss;

    ss << fs.rdbuf();
    fs.close();

    // get directory info
    CurrentDir = file.parent_path();
    ParseString(ss);
}

void Osu::Beatmap::ParseString(std::stringstream &ss)
{
    std::string lineraw;
    std::string currentSection;

    while (std::getline(ss, lineraw)) {
        if (lineraw.empty() || starts_with(lineraw, "//") || starts_with(lineraw, " ") || starts_with(lineraw, "_")) {
            continue;
        }

        std::string line;
        {
            // remove comment from line
            size_t commentpos = lineraw.find("//");
            if (commentpos != std::string::npos) {
                line = lineraw.substr(0, commentpos);
            } else {
                line = lineraw;
            }

            // trim line
            line.erase(0, line.find_first_not_of(" "));
            line.erase(line.find_last_not_of(" ") + 1);
        }

        if (starts_with(line, "osu file format")) {
            PeppyFormat = line.substr(line.find("v") + 1);
            continue;
        }

        if (starts_with(line, "[") && ends_with(line, "]")) {
            currentSection = line.substr(1, line.length() - 2);
            continue;
        }

        if (currentSection == "General") {
            if (starts_with(line, "AudioFilename")) {
                AudioFilename = line.substr(line.find(":") + 1);
                AudioFilename.erase(0, AudioFilename.find_first_not_of(" "));
            } else if (starts_with(line, "AudioLeadIn")) {
                AudioLeadIn = std::stoi(line.substr(line.find(":") + 1));
            } else if (starts_with(line, "PreviewTime")) {
                PreviewTime = std::stoi(line.substr(line.find(":") + 1));
            } else if (starts_with(line, "Countdown")) {
                Countdown = std::stoi(line.substr(line.find(":") + 1));
            } else if (starts_with(line, "SampleSet")) {
                SampleSet = line.substr(line.find(":") + 1);
            } else if (starts_with(line, "StackLeniency")) {
                StackLeniency = std::stof(line.substr(line.find(":") + 1));
            } else if (starts_with(line, "Mode")) {
                Mode = std::stoi(line.substr(line.find(":") + 1));
            } else if (starts_with(line, "LetterboxInBreaks")) {
                LetterboxInBreaks = line.substr(line.find(":") + 1) == "1";
            } else if (starts_with(line, "WidescreenStoryboard")) {
                WidescreenStoryboard = line.substr(line.find(":") + 1) == "1";
            } else if (starts_with(line, "SpecialStyle")) {
                SpecialStyle = line.substr(line.find(":") + 1) == "1";
            }

            continue;
        }

        if (currentSection == "Metadata") {
            if (starts_with(line, "Title")) {
                Title = line.substr(line.find(":") + 1);
            } else if (starts_with(line, "TitleUnicode")) {
                TitleUnicode = line.substr(line.find(":") + 1);
            } else if (starts_with(line, "Artist")) {
                Artist = line.substr(line.find(":") + 1);
            } else if (starts_with(line, "ArtistUnicode")) {
                ArtistUnicode = line.substr(line.find(":") + 1);
            } else if (starts_with(line, "Creator")) {
                Creator = line.substr(line.find(":") + 1);
            } else if (starts_with(line, "Version")) {
                Version = line.substr(line.find(":") + 1);
            } else if (starts_with(line, "Source")) {
                Source = line.substr(line.find(":") + 1);
            } else if (starts_with(line, "Tags")) {
                Tags = line.substr(line.find(":") + 1);
            } else if (starts_with(line, "BeatmapID")) {
                BeatmapID = std::stoi(line.substr(line.find(":") + 1));
            } else if (starts_with(line, "BeatmapSetID")) {
                BeatmapSetID = std::stoi(line.substr(line.find(":") + 1));
            }

            continue;
        }

        if (currentSection == "Difficulty") {
            if (starts_with(line, "HPDrainRate")) {
                HPDrainRate = std::stof(line.substr(line.find(":") + 1));
            } else if (starts_with(line, "CircleSize")) {
                CircleSize = std::stof(line.substr(line.find(":") + 1));
            } else if (starts_with(line, "OverallDifficulty")) {
                OverallDifficulty = std::stof(line.substr(line.find(":") + 1));
            } else if (starts_with(line, "ApproachRate")) {
                ApproachRate = std::stof(line.substr(line.find(":") + 1));
            } else if (starts_with(line, "SliderMultiplier")) {
                SliderMultiplier = std::stof(line.substr(line.find(":") + 1));
            } else if (starts_with(line, "SliderTickRate")) {
                SliderTickRate = std::stof(line.substr(line.find(":") + 1));
            }

            continue;
        }

        if (currentSection == "Events") {
            std::vector<std::string> event;
            {
                std::string_view view = line;
                while (!view.empty()) {
                    auto pos = view.find(',');
                    event.push_back(std::string(view.substr(0, pos)));
                    view.remove_prefix(pos != view.npos ? pos + 1 : view.size());
                }
            }

            OsuEvent ev = {};
            try {
                ev.StartTime = std::stof(event[1]);
            } catch (const std::invalid_argument &) {
                ev.StartTime = 0; // WHY TF people set invalid things on invalid row
            }

            // background
            if (event[0] == "0") {
                ev.Type = OsuEventType::Background;
            }

            // video
            else if (event[0] == "Video" || event[0] == "1") {
                ev.Type = OsuEventType::Videos;
            }

            // break
            else if (event[0] == "Break" || event[0] == "2") {
                ev.Type = OsuEventType::Break;
            }

            // sample
            else if (event[0] == "Sample" || event[0] == "5") {
                ev.Type = OsuEventType::Sample;
            }

            for (int i = 2; i < event.size(); i++) {
                std::string copy = event[i];
                if (starts_with(copy, "\"") && ends_with(copy, "\"")) {
                    copy = copy.substr(1, copy.size() - 2);
                }

                ev.params.push_back(copy);
            }

            Events.push_back(ev);
        }

        if (currentSection == "TimingPoints") {
            std::vector<std::string> timingPoint;
            {
                std::string_view view = line;
                while (!view.empty()) {
                    auto pos = view.find(',');
                    timingPoint.push_back(std::string(view.substr(0, pos)));
                    view.remove_prefix(pos != view.npos ? pos + 1 : view.size());
                }
            }

            if (timingPoint.size() < 8) {
                Logs::Puts("[osu::TimingPoints] Syntax error: %s", line.c_str());
                continue;
            }

            OsuTimingPoint tp = {};
            try {
                tp.Offset = std::stof(timingPoint[0]);
                tp.BeatLength = std::stof(timingPoint[1]);
                tp.TimeSignature = std::stof(timingPoint[2]);
                tp.SampleSet = std::stoi(timingPoint[3]);
                tp.SampleIndex = std::stoi(timingPoint[4]);
                tp.Volume = std::stoi(timingPoint[5]);
                tp.Inherited = std::stoi(timingPoint[6]) == 1;
                tp.KiaiMode = std::stoi(timingPoint[7]);
            } catch (const std::invalid_argument &) {
                Logs::Puts("[osu::TimingPoints] Syntax error: %s", line.c_str());
                continue;
            }

            TimingPoints.push_back(tp);
            continue;
        }

        if (currentSection == "HitObjects") {
            std::vector<std::string> hitObject;
            {
                std::string_view view = line;
                while (!view.empty()) {
                    auto pos = view.find(',');
                    hitObject.push_back(std::string(view.substr(0, pos)));
                    view.remove_prefix(pos != view.npos ? pos + 1 : view.size());
                }
            }

            if (hitObject.size() < 4) {
                Logs::Puts("[osu::HitObjects] Syntax error: %s", line.c_str());
                continue;
            }

            OsuHitObject ho;
            ho.X = std::stof(hitObject[0]);
            ho.Y = std::stof(hitObject[1]);
            ho.StartTime = std::stoi(hitObject[2]);
            ho.Type = std::stoi(hitObject[3]);
            ho.HitSound = std::stoi(hitObject[4]);
            ho.Additions = "0:0:0:0:";
            ho.KeysoundIndex = -1;

            if (ho.Type == 128) {
                std::string value = hitObject[5].substr(0, hitObject[5].find(":"));
                ho.EndTime = std::stoi(value);
            } else {
                ho.EndTime = -1;
            }

            if (hitObject.size() > 5) {
                std::vector<std::string> additions;
                {
                    std::string_view view = hitObject[5];
                    while (!view.empty()) {
                        auto pos = view.find(':');
                        additions.push_back(std::string(view.substr(0, pos)));
                        view.remove_prefix(pos != view.npos ? pos + 1 : view.size());
                    }
                }

                auto volumeField = ho.Type & 128 ? 4 : 3;
                if (additions.size() > volumeField) {
                    ho.Volume = std::max(0, std::stoi(additions[volumeField]));
                }

                auto keysoundField = volumeField + 1;
                if (additions.size() > keysoundField && additions[keysoundField].size() > 0) {
                    ho.KeysoundIndex = GetCustomSampleIndex(additions[keysoundField]);
                }
            }

            HitObjects.push_back(ho);
            continue;
        }
    }

    bIsValid = true;
}

int Osu::Beatmap::GetCustomSampleIndex(std::string path)
{
    for (int i = 0; i < HitSamples.size(); i++) {
        if (HitSamples[i] == path) {
            return i;
        }
    }

    HitSamples.push_back(path);
    return (int)HitSamples.size() - 1;
}

bool Osu::Beatmap::IsValid()
{
    return bIsValid;
}
