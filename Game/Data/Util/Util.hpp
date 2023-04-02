#pragma once
#include <string>
#include <vector>
#include <sstream>

std::vector<std::string> splitString(std::string& input, char delimeter);

std::vector<std::string> splitString(std::stringstream& input, char delimiter);

std::vector<std::string> splitString(std::stringstream& input);

std::string removeComment(const std::string& input);

std::string mergeVectorWith(std::vector<std::string>& vec, int starsAt = 0);

std::string mergeVector(std::vector<std::string>& vec, int starsAt = 0);

uint64_t Base36_Decode(const std::string& str);