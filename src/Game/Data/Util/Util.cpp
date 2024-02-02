/*
 * MIT License
 *
 * Copyright (c) 2023 Estrol Mendex
 * See the LICENSE file in the root of this project for details.
 */

#define _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING

#include "Util.hpp"
#include <algorithm>
#include <cstring>
#include <numeric>
#if _WIN32
#include <Windows.h>
#endif
#include <cerrno>
#include <cmath>
#include <codecvt>
#include <iconv.h>
#include <string>

std::vector<std::string> splitString(std::string &input, char delimeter)
{
    std::stringstream ss(input);
    return splitString(ss, delimeter);
}

std::vector<std::string> splitString(const std::string &input, char delimeter)
{
    std::stringstream ss(input);
    return splitString(ss, delimeter);
}

std::vector<std::string> splitString(std::stringstream &input, char delimiter)
{
    std::vector<std::string> result;
    std::string              line;
    while (std::getline(input, line, delimiter)) {
        result.push_back(line);
    }

    return result;
}

std::vector<std::string> splitString(std::stringstream &input)
{
    std::vector<std::string> result;
    std::string              line;
    while (std::getline(input, line)) {
        result.push_back(line);
    }

    return result;
}

std::string removeComment(const std::string &input)
{
    std::string result = input.substr(1);
    size_t      pos = result.find("//");
    if (pos != std::string::npos) {
        result = result.substr(0, pos);
    }

    return result;
}

std::string mergeVectorWith(std::vector<std::string> &vec, int starsAt)
{
    return std::accumulate(vec.begin() + starsAt, vec.end(), std::string{},
                           [](std::string acc, std::string str) {
                               return acc.empty() ? acc += str : acc += " " + str;
                           });
}

std::string mergeVector(std::vector<std::string> &vec, int starsAt)
{
    return std::accumulate(vec.begin() + starsAt, vec.end(), std::string{},
                           [](std::string acc, std::string str) {
                               return acc += str;
                           });
}

uint64_t Base36_Decode(const std::string &str)
{
    std::string CharList = "0123456789abcdefghijklmnopqrstuvwxyz";
    std::string reversed = str;
    std::reverse(reversed.begin(), reversed.end());
    std::transform(reversed.begin(), reversed.end(), reversed.begin(), ::tolower);

    uint64_t result = 0;
    int      pos = 0;

    for (char c : reversed) {
        result += (uint64_t)(CharList.find(c) * pow(36, pos));
        pos++;
    }

    return result;
}

std::string Base36_Encode(uint64_t num)
{
    static const std::string digits = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    std::stringstream        ss;
    std::string              result;

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

void flipArray(uint8_t *arr, size_t size)
{
    uint8_t *start = arr;
    uint8_t *end = arr + size - 1;

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

std::string CodepageToUtf8(const char *string, size_t str_len, const char *encoding)
{
    std::string result;
    iconv_t     conv = iconv_open("UTF-8", encoding);
    if (conv == (iconv_t)-1) {
        return (const char *)(u8"<encoding error>");
    }

    size_t            inbytesleft = str_len;
    size_t            outbytesleft = str_len * 4;
    char             *inbuf = (char *)string;
    std::vector<char> outbuf;
    outbuf.resize(outbytesleft, 0);

    char *outbufptr = outbuf.data();

    if (iconv(conv, &inbuf, &inbytesleft, &outbufptr, &outbytesleft) == (size_t)-1) {
        iconv_close(conv);

        // return whatever we have left, even if it's not valid
        size_t      len = strlen(outbuf.data());
        std::string remaining = std::string((const char *)outbuf.data(), len);

        return remaining;
    }

    result = std::string(outbuf.data(), str_len * 4 - outbytesleft);
    iconv_close(conv);

    return result;
}

bool starts_with(std::string &str, std::string_view prefix)
{
    return str.rfind(prefix, 0) == 0;
}

bool ends_with(std::string &str, std::string_view suffix)
{
    if (suffix.size() > str.size()) {
        return false;
    }

    return std::equal(suffix.rbegin(), suffix.rend(), str.rbegin());
}
