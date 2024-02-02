/*
 * MIT License
 *
 * Copyright (c) 2023 Estrol Mendex
 * See the LICENSE file in the root of this project for details.
 */

#pragma once
#include <filesystem>
#include <string>
#include <vector>

struct O2Sample
{
    int8_t               FileName[32];
    uint32_t             RefValue;
    std::vector<uint8_t> AudioData;
};

class OJM
{
public:
    ~OJM();

    void Load(std::filesystem::path &fileName);
    bool IsValid();

    std::vector<O2Sample> Samples;

private:
    void LoadM30Data(std::fstream &fs);
    void LoadOJMData(std::fstream &fs, bool encrypted);

    bool m_valid;
};