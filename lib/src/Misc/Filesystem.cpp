/*
 * MIT License
 *
 * Copyright (c) 2023 Estrol Mendex
 * See the LICENSE file in the root of this project for details.
 */

#include <Exceptions/EstException.h>
#include <Misc/Filesystem.h>
#include <fstream>
using namespace Misc;
using namespace std::filesystem;

std::fstream InternalOpenFile(path path)
{
    if (!exists(path)) {
        throw Exceptions::EstException("Failed to open file: %s", path.string().c_str());
    }

    std::fstream fs(path, std::ios::in | std::ios::binary);
    if (!fs.is_open()) {
        throw Exceptions::EstException("Failed to open file: %s", path.string().c_str());
    }

    return fs;
}

std::fstream InternalWriteFile(path path)
{
    if (!exists(path)) {
        throw Exceptions::EstException("Failed to open file: %s", path.string().c_str());
    }

    std::fstream fs(path, std::ios::out | std::ios::binary);
    if (!fs.is_open()) {
        throw Exceptions::EstException("Failed to open file: %s", path.string().c_str());
    }

    return fs;
}

std::vector<uint8_t> InternalReadFile(path path)
{
    auto fs = InternalOpenFile(path);

    fs.seekg(0, std::ios::end);
    size_t size = fs.tellg();
    fs.seekg(0, std::ios::beg);

    std::vector<uint8_t> buf(size);
    fs.read((char *)buf.data(), size);
    fs.close();

    return buf;
}

std::vector<uint8_t> Filesystem::ReadFile(path path)
{
    auto buffer = InternalReadFile(path);
    return buffer;
}

std::vector<uint16_t> Filesystem::ReadFile16(path path)
{
    auto buffer = InternalReadFile(path);

    bool   needsPadding = false;
    size_t sizeInBytes = buffer.size();

    if (sizeInBytes % 2 != 0) {
        sizeInBytes++;
        needsPadding = true;
    }

    size_t sizeInU16 = sizeInBytes / 2;

    std::vector<uint16_t> buf(sizeInU16);
    memcpy(buf.data(), buffer.data(), sizeInBytes - (needsPadding ? 1 : 0));

    if (needsPadding) {
        buf.back() = 0;
    }

    return buf;
}

std::string Filesystem::ReadFileString(path path)
{
    auto buffer = InternalReadFile(path);

    return std::string((char *)buffer.data(), buffer.size());
}

void Filesystem::WriteFile(path path, const std::vector<uint8_t> &buffer)
{
    auto fs = InternalWriteFile(path);

    fs.write((char *)buffer.data(), buffer.size());
    fs.close();
}

void Filesystem::WriteFile16(path path, const std::vector<uint16_t> &buffer)
{
    auto fs = InternalWriteFile(path);

    fs.write((char *)buffer.data(), buffer.size());
    fs.close();
}

void Filesystem::WriteFileString(path path, const std::string &buffer)
{
    auto fs = InternalWriteFile(path);

    fs.write(buffer.c_str(), buffer.size());
    fs.close();
}