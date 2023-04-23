#include "SkinConfig.hpp"
#include "../Data/Util/mINI.h"
#include "../Data/Util/Util.hpp"
#include <filesystem>
#include <iostream>

SkinConfig::SkinConfig(std::string filePath) {
	auto path = std::filesystem::current_path().string();

	if (!filePath.starts_with(path)) {
		filePath = path + filePath;
	}

	std::filesystem::path current = std::filesystem::path(filePath).parent_path();
	mINI::INIFile f(filePath);
	mINI::INIStructure ini;
	f.read(ini);

	for (auto const& [key, value] : ini["Numerics"]) {
		auto split = splitString(value, ',');

		NumericValue e = {};
		e.X = std::stoi(split[0]);
		e.Y = std::stoi(split[1]);

		if (split.size() > 2) {
			e.MaxDigit = std::stoi(split[2]);
		}
		else {
			e.MaxDigit = 0;
			e.Direction = 0;
			e.FillWithZero = false;
		}

		if (split.size() > 3) {
			auto direction = split[3];
			std::transform(direction.begin(), direction.end(), direction.begin(), ::tolower);

			if (direction == "mid") e.Direction = 0;
			else if (direction == "left") e.Direction = -1;
			else if (direction == "right") e.Direction = 1;
		}
		else {
			e.Direction = 0;
			e.FillWithZero = false;
		}

		if (split.size() > 4) {
			auto direction = split[4];
			std::transform(direction.begin(), direction.end(), direction.begin(), ::tolower);

			if (direction == "true") e.FillWithZero = true;
			else if (direction == "false") e.FillWithZero = false;
			else if (direction == "1") e.FillWithZero = true;
			else e.FillWithZero = false;
		}
		else {
			e.FillWithZero = false;
		}

		m_numericValues[key] = std::move(e);
	}

	for (auto const& [key, value] : ini["Positions"]) {
		auto split = splitString(value, ',');

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

		m_positionValues[key] = std::move(e);
	}

	for (auto const& [key, value] : ini["Sprites"]) {
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

		m_spriteValues[key] = std::move(e);
	}
}

SkinConfig::~SkinConfig() {
	m_positionValues.clear();
	m_numericValues.clear();
}

PositionValue& SkinConfig::GetPosition(std::string key) {
	std::transform(key.begin(), key.end(), key.begin(), ::tolower);

	if (m_positionValues.find(key) == m_positionValues.end()) {
		throw std::runtime_error("Position key not found: " + key);
	}

	return m_positionValues[key];
}

NumericValue& SkinConfig::GetNumeric(std::string key) {
	std::transform(key.begin(), key.end(), key.begin(), ::tolower);

	if (m_numericValues.find(key) == m_numericValues.end()) {
		throw std::runtime_error("Numeric key not found: " + key);
	}

	return m_numericValues[key];
}

SpriteValue& SkinConfig::GetSprite(std::string key) {
	std::transform(key.begin(), key.end(), key.begin(), ::tolower);

	if (m_spriteValues.find(key) == m_spriteValues.end()) {
		throw std::runtime_error("Sprite key not found: " + key);
	}

	return m_spriteValues[key];
}
