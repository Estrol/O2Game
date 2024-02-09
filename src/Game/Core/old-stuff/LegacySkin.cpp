/*
 * MIT License
 *
 * Copyright (c) 2023 Estrol Mendex
 * See the LICENSE file in the root of this project for details.
 */

#include "LegacySkin.h"

#include <Exceptions/EstException.h>

#include <filesystem>
#include <iostream>

#include "../../Data/Util/Util.hpp"
#include "Misc/ini.h"

LegacySkin::LegacySkin(std::string filePath, int keyCount)
{
    auto path = std::filesystem::current_path().string();

    if (!starts_with(filePath, path)) {
        filePath = path + filePath;
    }

    Load(path, keyCount);
}

LegacySkin::LegacySkin(std::filesystem::path path, int keyCount)
{
    Load(path, keyCount);
}

void LegacySkin::Load(std::filesystem::path path, int keyCount)
{
    std::filesystem::path current = path.parent_path();
    if (!std::filesystem::exists(path)) {
        throw Exceptions::EstException("File does not exist: %s", path.string().c_str());
    }

    Misc::mINI::INIFile      f(path.string());
    Misc::mINI::INIStructure ini;
    f.read(ini);

    for (auto const &[key, value] : ini["Numerics"]) {
        auto rows = splitString(value, '|');
        for (auto &value2 : rows) {
            auto split = splitString(value2, ',');

            NumericValue e = {};
            e.X = std::stoi(split[0]);
            e.Y = std::stoi(split[1]);

            if (split.size() > 2) {
                e.MaxDigit = std::stoi(split[2]);
            } else {
                e.MaxDigit = 0;
                e.Direction = 0;
                e.FillWithZero = false;
            }

            if (split.size() > 3) {
                auto direction = split[3];
                std::transform(direction.begin(), direction.end(), direction.begin(), ::tolower);

                if (direction == "mid")
                    e.Direction = 0;
                else if (direction == "left")
                    e.Direction = -1;
                else if (direction == "right")
                    e.Direction = 1;
            } else {
                e.Direction = 0;
                e.FillWithZero = false;
            }

            if (split.size() > 4) {
                auto direction = split[4];
                std::transform(direction.begin(), direction.end(), direction.begin(), ::tolower);

                if (direction == "true")
                    e.FillWithZero = true;
                else if (direction == "false")
                    e.FillWithZero = false;
                else if (direction == "1")
                    e.FillWithZero = true;
                else
                    e.FillWithZero = false;
            } else {
                e.FillWithZero = false;
            }

            m_numericValues[key].push_back(std::move(e));
        }
    }

    std::string keyName = "positions";
    if (keyCount != -1) {
        keyName += "#" + std::to_string(keyCount);
    }

    for (auto const &[key, value] : ini[keyName]) {
        auto rows = splitString(value, '|');
        for (auto &value2 : rows) {
            auto split = splitString(value2, ',');

            PositionValue e = {};
            e.X = std::stoi(split[0]);
            e.Y = std::stoi(split[1]);

            if (split.size() > 2) {
                e.AnchorPointX = std::stof(split[2]);
            }

            if (split.size() > 3) {
                e.AnchorPointY = std::stof(split[3]);
            }

            if (split.size() > 4) {
                auto splitRGB = splitString(split[4], ':');
                e.RGB[0] = std::stoi(splitRGB[0]);
                e.RGB[1] = std::stoi(splitRGB[1]);
                e.RGB[2] = std::stoi(splitRGB[2]);
            }

            m_positionValues[key].push_back(std::move(e));
        }
    }

    std::string keyName1 = "notes";
    if (keyCount != -1) {
        keyName1 += "#" + std::to_string(keyCount);
    }

    for (auto const &[key, value] : ini[keyName1]) {
        auto split = splitString(value, ',');

        NoteValue e = {};
        e.numOfFiles = std::stoi(split[0]);
        e.fileName = split[1];

        m_noteValues[key] = std::move(e);
    }

    for (auto const &[key, value] : ini["Sprites"]) {
        auto split = splitString(value, ',');

        SpriteValue e = {};
        e.numOfFrames = std::stoi(split[0]);
        e.X = std::stoi(split[1]);
        e.Y = std::stoi(split[2]);

        if (split.size() > 3) {
            e.AnchorPointX = std::stof(split[3]);
        }

        if (split.size() > 4) {
            e.AnchorPointY = std::stof(split[4]);
        }

        if (split.size() > 5) {
            e.FrameTime = std::stof(split[5]);
        }

        m_spriteValues[key] = std::move(e);
    }

    for (auto const &[key, value] : ini["rects"]) {
        auto rows = splitString(value, '|');
        for (auto &value2 : rows) {
            auto split = splitString(value2, ',');

            RectInfo e = {};
            e.X = std::stoi(split[0]);
            e.Y = std::stoi(split[1]);
            e.Width = std::stoi(split[2]);
            e.Height = std::stoi(split[3]);

            m_rectValues[key].push_back(std::move(e));
        }
    }
}

LegacySkin::~LegacySkin()
{
    m_positionValues.clear();
    m_numericValues.clear();
}

std::vector<PositionValue> &LegacySkin::GetPosition(std::string key)
{
    std::transform(key.begin(), key.end(), key.begin(), ::tolower);

    if (m_positionValues.find(key) == m_positionValues.end()) {
        throw Exceptions::EstException("Position key not found: %s", key.c_str());
    }

    return m_positionValues[key];
}

std::vector<RectInfo> &LegacySkin::GetRect(std::string key)
{
    std::transform(key.begin(), key.end(), key.begin(), ::tolower);

    if (m_rectValues.find(key) == m_rectValues.end()) {
        throw Exceptions::EstException("Rect key not found: %s", key.c_str());
    }

    return m_rectValues[key];
}

NoteValue &LegacySkin::GetNote(std::string key)
{
    std::transform(key.begin(), key.end(), key.begin(), ::tolower);

    if (m_noteValues.find(key) == m_noteValues.end()) {
        throw Exceptions::EstException("Sprite key not found: %s", key.c_str());
    }

    return m_noteValues[key];
}

std::vector<NumericValue> &LegacySkin::GetNumeric(std::string key)
{
    std::transform(key.begin(), key.end(), key.begin(), ::tolower);

    if (m_numericValues.find(key) == m_numericValues.end()) {
        throw Exceptions::EstException("Numeric key not found: %s", key.c_str());
    }

    return m_numericValues[key];
}

SpriteValue &LegacySkin::GetSprite(std::string key)
{
    std::transform(key.begin(), key.end(), key.begin(), ::tolower);

    if (m_spriteValues.find(key) == m_spriteValues.end()) {
        throw Exceptions::EstException("Sprite key not found: %s", key);
    }

    return m_spriteValues[key];
}
