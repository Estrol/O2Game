#pragma once
#include <filesystem>
#include <vector>

namespace MusicListMaker {
    std::vector<std::filesystem::path> Prepare(std::filesystem::path path);
    void Insert(std::filesystem::path song_file);
}