#pragma once
#include <string>
#include <vector>
#include <sstream>
#include <algorithm>

std::vector<std::string> splitString(std::string& input, char delimeter);

std::vector<std::string> splitString(const std::string& input, char delimeter);

std::vector<std::string> splitString(std::stringstream& input, char delimiter);

std::vector<std::string> splitString(std::stringstream& input);

std::string removeComment(const std::string& input);

std::string mergeVectorWith(std::vector<std::string>& vec, int starsAt = 0);

std::string mergeVector(std::vector<std::string>& vec, int starsAt = 0);

uint64_t Base36_Decode(const std::string& str);

template <typename T, typename Predicate>
std::vector<T> FindWhere(std::vector<T>& vec, Predicate func) {
	std::vector<T> result;
	std::copy_if(result.begin(), result.end(), std::back_inserter(result), func);
	return result;
}

template <typename T, typename T2, typename Predicate>
T FindValue(std::vector<T2> vec, Predicate func) {
	auto it = std::find_if(vec.begin(), vec.end(), func);
	if (it != vec.end()) {
		return it->second;
	}
	
	return T();
}