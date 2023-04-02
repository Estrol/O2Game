#include "Util.hpp"
#include <numeric>
#include <algorithm>

std::vector<std::string> splitString(std::string& input, char delimeter) {
	std::stringstream ss(input);
	return splitString(ss, delimeter);
}

std::vector<std::string> splitString(std::stringstream& input, char delimiter) {
	std::vector<std::string> result;
	std::string line;
	while (std::getline(input, line, delimiter)) {
		result.push_back(line);
	}

	return result;
}

std::vector<std::string> splitString(std::stringstream& input) {
	std::vector<std::string> result;
	std::string line;
	while (std::getline(input, line)) {
		result.push_back(line);
	}

	return result;
}

std::string removeComment(const std::string& input) {
	std::string result = input.substr(1);
	size_t pos = result.find("//");
	if (pos != std::string::npos) {
		result = result.substr(0, pos);
	}

	return result;
}

std::string mergeVectorWith(std::vector<std::string>& vec, int starsAt) {
	return std::accumulate(vec.begin() + starsAt, vec.end(), std::string{},
		[](std::string acc, std::string str) {
			return acc.empty() ? acc += str : acc += " " + str;
		});
}

std::string mergeVector(std::vector<std::string>& vec, int starsAt) {
	return std::accumulate(vec.begin() + starsAt, vec.end(), std::string{},
		[](std::string acc, std::string str) {
			return acc += str;
		});
}

uint64_t Base36_Decode(const std::string& str) {
	std::string CharList = "0123456789abcdefghijklmnopqrstuvwxyz";
	std::string reversed = str;
	std::reverse(reversed.begin(), reversed.end());
	std::transform(reversed.begin(), reversed.end(), reversed.begin(), ::tolower);

	uint64_t result = 0;
	int pos = 0;

	for (char c : reversed) {
		result += CharList.find(c) * pow(36, pos);
		pos++;
	}

	return result;
}