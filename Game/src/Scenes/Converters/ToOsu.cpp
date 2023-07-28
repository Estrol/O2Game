#include "ToOsu.hpp"
#include <fstream>
#include "../../Data/osu.hpp"
#include "../../Data/Util/Util.hpp"

bool Converters::SaveTo(O2::OJN* ojn, const char* path) {
	auto converterPath = std::filesystem::current_path() / "Converted";
	if (!std::filesystem::exists(converterPath)) {
		std::filesystem::create_directories(converterPath);
	}

	std::string ojnFile = "o2ma" + std::to_string(ojn->Header.songid);
	std::vector<std::string> fileName = {
		ojnFile + "-EX.osu",
		ojnFile + "-NX.osu",
		ojnFile + "-HX.osu",
	};

	for (auto& [index, difficulty] : ojn->Difficulties) {
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

		auto title = CodepageToUtf8(ojn->Header.title, sizeof(ojn->Header.title), 949);
		fs.write((char*)title.c_str(), title.size());

		fs << "\n";

		fs << "TitleUnicode: ";
		fs.write((char*)title.c_str(), title.size());

		fs << "\n";

		fs << "Artist: ";

		auto artist = CodepageToUtf8(ojn->Header.artist, sizeof(ojn->Header.artist), 949);
		fs.write((char*)artist.c_str(), artist.size());

		fs << "\n";

		fs << "ArtistUnicode: ";
		fs.write((char*)artist.c_str(), artist.size());

		fs << "\n";

		fs << "Creator: ";

		auto creator = CodepageToUtf8(ojn->Header.noter, sizeof(ojn->Header.noter), 949);
		fs.write((char*)creator.c_str(), creator.size());

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
		
		for (auto& sample : difficulty.AutoSamples) {
			fs << "5," << sample.StartTime << ",0,\"Sample-" << sample.SampleRefId << ".ogg\"\n";
		}
	}

    return true;
}
