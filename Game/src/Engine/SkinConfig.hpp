#pragma once
#include <filesystem>
#include <string>
#include <unordered_map>
#include <vector>

#include "../Resources/SkinStructs.hpp"

class SkinConfig
{
public:
    SkinConfig() = default;
    SkinConfig(std::string filePath, int keyCount);
    SkinConfig(std::filesystem::path path, int keyCount);
    ~SkinConfig();

    std::vector<NumericValue>  &GetNumeric(std::string key);
    std::vector<PositionValue> &GetPosition(std::string key);
    std::vector<RectInfo>      &GetRect(std::string key);
    NoteValue                  &GetNote(std::string key);
    SpriteValue                &GetSprite(std::string key);

private:
    void Load(std::filesystem::path path, int keyCount);

    std::unordered_map<std::string, std::vector<NumericValue>>  m_numericValues;
    std::unordered_map<std::string, std::vector<PositionValue>> m_positionValues;
    std::unordered_map<std::string, SpriteValue>                m_spriteValues;
    std::unordered_map<std::string, std::vector<RectInfo>>      m_rectValues;
    std::unordered_map<std::string, NoteValue>                  m_noteValues;
};