/*
 * MIT License
 *
 * Copyright (c) 2023 Estrol Mendex
 * See the LICENSE file in the root of this project for details.
 */

#ifndef __CONFIGURATION_H_
#define __CONFIGURATION_H_

#include <string>

namespace Configuration {
    // Set default configuration for each time the configuration file is reset
    void SetDefaultConfiguration(std::string conf);

    // Reset configuration file to default
    void ResetConfiguration();

    // Set string value on key at section
    void Set(std::string section, std::string key, std::string value);

    // Set int value on key at section
    void SetInt(std::string section, std::string key, int value);

    // Set float value on key at section
    void SetFloat(std::string section, std::string key, float value);

    // Set bool value on key at section
    void SetBool(std::string section, std::string key, bool value);

    // Get string value on key at section
    std::string Get(std::string section, std::string key);

    // Get int value on key at section
    int GetInt(std::string section, std::string key);

    // Get float value on key at section
    float GetFloat(std::string section, std::string key);

    // Get bool value on key at section
    bool GetBool(std::string section, std::string key);

} // namespace Configuration

#endif