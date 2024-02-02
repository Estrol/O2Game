/*
 * MIT License
 *
 * Copyright (c) 2023 Estrol Mendex
 * See the LICENSE file in the root of this project for details.
 */

#pragma once
#include <algorithm>
#include <sstream>
#include <stdint.h>
#include <string>
#include <vector>

std::vector<std::string> splitString(std::string &input, char delimeter);

std::vector<std::string> splitString(const std::string &input, char delimeter);

std::vector<std::string> splitString(std::stringstream &input, char delimiter);

std::vector<std::string> splitString(std::stringstream &input);

std::string removeComment(const std::string &input);

std::string mergeVectorWith(std::vector<std::string> &vec, int starsAt = 0);

std::string mergeVector(std::vector<std::string> &vec, int starsAt = 0);

uint64_t Base36_Decode(const std::string &str);

std::string Base36_Encode(uint64_t num);

void flipArray(uint8_t *arr, size_t size);

template <typename T, typename Predicate>
std::vector<T> FindWhere(std::vector<T> &vec, Predicate func)
{
    std::vector<T> result;
    std::copy_if(result.begin(), result.end(), std::back_inserter(result), func);
    return result;
}

template <typename T, typename T2, typename Predicate>
T FindValue(std::vector<T2> vec, Predicate func)
{
    auto it = std::find_if(vec.begin(), vec.end(), func);
    if (it != vec.end()) {
        return it->second;
    }

    return T();
}

std::string CodepageToUtf8(const char *string, size_t len, const char *encoding);

bool starts_with(std::string &str, std::string_view prefix);
bool ends_with(std::string &str, std::string_view suffix);

template <typename T>
T accumulate(const std::vector<T> &vec)
{
    T sum = 0;
    for (auto &v : vec) {
        sum += v;
    }

    return sum;
}

template <typename T>
T accumulate(const T *beg, const T *end, size_t sz = 0)
{
    if (!sz) {
        sz = end - beg;
    }

    T sum = 0;
    for (int i = 0; i < sz; i++) {
        sum += beg[i];
    }

    return sum;
}

template <typename T>
T lerp(T a, T b, float t)
{
    float fA = static_cast<float>(a);
    float fB = static_cast<float>(b);

    float value = a + (b - a) * t;

    return static_cast<T>(value);
}