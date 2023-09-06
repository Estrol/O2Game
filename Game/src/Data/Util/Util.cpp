#define _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING

#include "Util.hpp"
#include <numeric>
#include <algorithm>
#if _WIN32
#include <Windows.h>
#endif
#include <string>
#include <codecvt>
#include <cmath>

std::vector<std::string> splitString(std::string& input, char delimeter) {
	std::stringstream ss(input);
	return splitString(ss, delimeter);
}

std::vector<std::string> splitString(const std::string& input, char delimeter) {
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
		result += (uint64_t)(CharList.find(c) * pow(36, pos));
		pos++;
	}

	return result;
}

std::string Base36_Encode(uint64_t num) {
	static const std::string digits = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	std::stringstream ss;
	std::string result;

	while (num > 0) {
		ss << digits[num % 36];
		num /= 36;
	}

	result = ss.str();
	std::reverse(result.begin(), result.end());

	if (result.length() == 1) {
		result = "0" + result;
	}

	return result;
}

void flipArray(uint8_t* arr, size_t size) {
	uint8_t* start = arr;
	uint8_t* end = arr + size - 1;

	while (start < end) {
		// flip the values using XOR bitwise operation
		*start ^= *end;
		*end ^= *start;
		*start ^= *end;

		// move the pointers inward
		start++;
		end--;
	}
}

std::u8string CodepageToUtf8(const char* string, size_t str_len, int codepage) {
#if _WIN32
	int size_needed = MultiByteToWideChar(codepage, 0, &string[0], (int)str_len, NULL, 0);
	std::vector<wchar_t> temp_string(size_needed);

	MultiByteToWideChar(codepage, 0, &string[0], (int)str_len, temp_string.data(), size_needed);

	int utf8_size_needed = WideCharToMultiByte(CP_UTF8, 0, temp_string.data(), size_needed, NULL, 0, NULL, NULL);
	std::vector<char8_t> result(utf8_size_needed);

	WideCharToMultiByte(CP_UTF8, 0, temp_string.data(), size_needed, (LPSTR)&result[0], utf8_size_needed, NULL, NULL);

	std::u8string str_result = result.data();

	return str_result;
#else
	return std::u8string((const char8_t*)string, str_len);
#endif
}
