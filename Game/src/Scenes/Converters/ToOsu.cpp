#include "ToOsu.hpp"
#include "../../Data/Util/Util.hpp"
#include "../../Data/osu.hpp"
#include <fstream>

bool Converters::SaveTo(O2::OJN *ojn, const char *path)
{
    auto converterPath = std::filesystem::current_path() / "Converted";
    if (!std::filesystem::exists(converterPath)) {
        std::filesystem::create_directories(converterPath);
    }

    std::string              ojnFile = "o2ma" + std::to_string(ojn->Header.songid);
    std::vector<std::string> fileName = {
        ojnFile + "-EX.osu",
        ojnFile + "-NX.osu",
        ojnFile + "-HX.osu",
    };

    for (auto &[index, difficulty] : ojn->Difficulties) {
        std::fstream fs(converterPath / fileName[index], std::ios::binary | std::ios::out);

        if (!fs.is_open()) {
            return false;
        }

        fs << "osu file format v14\n\n";

        fs << "[General]\n";
        fs << "AudioFilename:\n";
        fs << "Mode: 3\n";
        fs << "AudioLeadIn: 0\n\n";

        fs << "[Metadata]\n";
        fs << "Title: ";

        auto title = CodepageToUtf8(ojn->Header.title, sizeof(ojn->Header.title), "euc-kr");
        fs.write((char *)title.c_str(), title.size());

        fs << "\n";

        fs << "TitleUnicode: ";
        fs.write((char *)title.c_str(), title.size());

        fs << "\n";

        fs << "Artist: ";

        auto artist = CodepageToUtf8(ojn->Header.artist, sizeof(ojn->Header.artist), "euc-kr");
        fs.write((char *)artist.c_str(), artist.size());

        fs << "\n";

        fs << "ArtistUnicode: ";
        fs.write((char *)artist.c_str(), artist.size());

        fs << "\n";

        fs << "Creator: ";

        auto creator = CodepageToUtf8(ojn->Header.noter, sizeof(ojn->Header.noter), "euc-kr");
        fs.write((char *)creator.c_str(), creator.size());

        fs << "\n";

        fs << "Version: 7K-" + std::to_string(index) + "\n";
        fs << "Source: O2Jam\n";

        fs << "Tags: o2jam o2ma" + std::to_string(ojn->Header.songid) + "\n";
        fs << "BeatmapID: 0\n";
        fs << "BeatmapSetID: -1\n\n";

        fs << "[Difficulty]\n";
        fs << "HPDrainRate: 7\n";
        fs << "CircleSize: 7\n";
        fs << "OverallDifficulty: 8\n";
        fs << "ApproachRate: 5\n";
        fs << "SliderMultiplier: 1.4\n";
        fs << "SliderTickRate: 1\n\n";

        fs << "[Events]\n";
        fs << "0,0\"background.jpg\"\n";

        for (auto &sample : difficulty.AutoSamples) {
            fs << "5," << sample.StartTime << ",0,\"Sample-" << sample.SampleRefId << ".ogg\"\n";
        }

        fs << "\n";

        fs << "[TimingPoints]\n";
        for (auto &timing : difficulty.Timings) {
            fs << timing.Time << "," << 60000.0 / timing.BPM << ",4,2,0,100,1,0\n";
        }

        fs << "\n";

        fs << "[HitObjects]\n";

        for (auto &note : difficulty.Notes) {
            if (!note.IsLN) {
                if (note.SampleRefId != -1) {
                    std::string sampleId = "Sample#" + std::to_string(note.SampleRefId) + ".ogg";
                    fs << note.StartTime << ",192,0,128,0,0:0:0:0:" << sampleId << "\n";
                }
            } else {
                fs << note.StartTime << ",192,0,128,0," << note.EndTime << ":0:0:0:\n";
            }
        }

        fs.close();
    }

    return true;
}
