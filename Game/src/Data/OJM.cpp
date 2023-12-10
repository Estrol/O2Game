#include "OJM.hpp"
#include "Util/Util.hpp"
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string.h>

constexpr int kM30Signature = 0x0030334D;
constexpr int kOMCSignature = 0x00434D4F;
constexpr int kOJMSignature = 0x004D4A4F;

const char MASK_NAMI[] = { 0x6E, 0x61, 0x6D, 0x69 };
const char MASK_0412[] = { 0x30, 0x34, 0x31, 0x32 };

void M30Xor(char *data, size_t sz, const char *xorKey)
{
    for (int i = 0; i + 3 < sz; i += 4) {
        data[i] ^= xorKey[0];
        data[i + 1] ^= xorKey[1];
        data[i + 2] ^= xorKey[2];
        data[i + 3] ^= xorKey[3];
    }
}

const unsigned char WeirdRearrangeTable[] = {
    0x10, 0x0E, 0x02, 0x09, 0x04, 0x00, 0x07, 0x01,
    0x06, 0x08, 0x0F, 0x0A, 0x05, 0x0C, 0x03, 0x0D,
    0x0B, 0x07, 0x02, 0x0A, 0x0B, 0x03, 0x05, 0x0D,
    0x08, 0x04, 0x00, 0x0C, 0x06, 0x0F, 0x0E, 0x10,
    0x01, 0x09, 0x0C, 0x0D, 0x03, 0x00, 0x06, 0x09,
    0x0A, 0x01, 0x07, 0x08, 0x10, 0x02, 0x0B, 0x0E,
    0x04, 0x0F, 0x05, 0x08, 0x03, 0x04, 0x0D, 0x06,
    0x05, 0x0B, 0x10, 0x02, 0x0C, 0x07, 0x09, 0x0A,
    0x0F, 0x0E, 0x00, 0x01, 0x0F, 0x02, 0x0C, 0x0D,
    0x00, 0x04, 0x01, 0x05, 0x07, 0x03, 0x09, 0x10,
    0x06, 0x0B, 0x0A, 0x08, 0x0E, 0x00, 0x04, 0x0B,
    0x10, 0x0F, 0x0D, 0x0C, 0x06, 0x05, 0x07, 0x01,
    0x02, 0x03, 0x08, 0x09, 0x0A, 0x0E, 0x03, 0x10,
    0x08, 0x07, 0x06, 0x09, 0x0E, 0x0D, 0x00, 0x0A,
    0x0B, 0x04, 0x05, 0x0C, 0x02, 0x01, 0x0F, 0x04,
    0x0E, 0x10, 0x0F, 0x05, 0x08, 0x07, 0x0B, 0x00,
    0x01, 0x06, 0x02, 0x0C, 0x09, 0x03, 0x0A, 0x0D,
    0x06, 0x0D, 0x0E, 0x07, 0x10, 0x0A, 0x0B, 0x00,
    0x01, 0x0C, 0x0F, 0x02, 0x03, 0x08, 0x09, 0x04,
    0x05, 0x0A, 0x0C, 0x00, 0x08, 0x09, 0x0D, 0x03,
    0x04, 0x05, 0x10, 0x0E, 0x0F, 0x01, 0x02, 0x0B,
    0x06, 0x07, 0x05, 0x06, 0x0C, 0x04, 0x0D, 0x0F,
    0x07, 0x0E, 0x08, 0x01, 0x09, 0x02, 0x10, 0x0A,
    0x0B, 0x00, 0x03, 0x0B, 0x0F, 0x04, 0x0E, 0x03,
    0x01, 0x00, 0x02, 0x0D, 0x0C, 0x06, 0x07, 0x05,
    0x10, 0x09, 0x08, 0x0A, 0x03, 0x02, 0x01, 0x00,
    0x04, 0x0C, 0x0D, 0x0B, 0x10, 0x05, 0x06, 0x0F,
    0x0E, 0x07, 0x09, 0x0A, 0x08, 0x09, 0x0A, 0x00,
    0x07, 0x08, 0x06, 0x10, 0x03, 0x04, 0x01, 0x02,
    0x05, 0x0B, 0x0E, 0x0F, 0x0D, 0x0C, 0x0A, 0x06,
    0x09, 0x0C, 0x0B, 0x10, 0x07, 0x08, 0x00, 0x0F,
    0x03, 0x01, 0x02, 0x05, 0x0D, 0x0E, 0x04, 0x0D,
    0x00, 0x01, 0x0E, 0x02, 0x03, 0x08, 0x0B, 0x07,
    0x0C, 0x09, 0x05, 0x0A, 0x0F, 0x04, 0x06, 0x10,
    0x01, 0x0E, 0x02, 0x03, 0x0D, 0x0B, 0x07, 0x00,
    0x08, 0x0C, 0x09, 0x06, 0x0F, 0x10, 0x05, 0x0A,
    0x04, 0x00
};

std::vector<char> WeirdRearrange(char *data, size_t sz)
{
    int len = (int)sz;
    int key = ((len % 17) << 4) + (len % 17);
    int blockSz = len / 17;

    std::vector<char> res(len);
    for (int i = 0; i < 17; i++) {
        int inOffset = blockSz * i;
        int outOffset = blockSz * WeirdRearrangeTable[key];

        memcpy(res.data() + outOffset, data + inOffset, blockSz);

        key++;
    }

    return res;
}

// weird tracking
static int accKeyByte = 0xFF;
static int accCounter = 0;

std::vector<char> XorDecrypt(std::vector<char> &data)
{
    int  tmp;
    char this_char;

    std::vector<char> result(data.size());
    for (int i = 0; i < data.size(); i++) {
        tmp = data[i];
        this_char = tmp;

        if (((accKeyByte << accCounter) & 0x80) != 0) {
            this_char = (char)~this_char;
        }

        result[i] = this_char;
        accCounter++;

        if (accCounter > 7) {
            accCounter = 0;
            accKeyByte = tmp;
        }
    }

    return result;
}

OJM::~OJM()
{
}

void OJM::Load(std::filesystem::path &fileName)
{
    if (!std::filesystem::exists(fileName)) {
        return;
    }

    std::fstream fs(fileName, std::ios::binary | std::ios::in);
    if (!fs.is_open()) {
        return;
    }

    int signature = 0;
    fs.read((char *)&signature, sizeof(int));

    switch (signature) {
        case kM30Signature:
        {
            LoadM30Data(fs);
            break;
        }

        case kOJMSignature:
        {
            LoadOJMData(fs, false);
            break;
        }

        case kOMCSignature:
        {
            LoadOJMData(fs, true);
            break;
        }

        default:
        {
            fs.close();
            return;
        }
    }

    std::sort(Samples.begin(), Samples.end(), [](const auto &a, const auto &b) {
        return a.RefValue < b.RefValue;
    });

    fs.close();
    m_valid = true;
}

bool OJM::IsValid()
{
    return m_valid;
}

void OJM::LoadM30Data(std::fstream &fs)
{
    struct M30Header
    {
        int fileFormatVersion;
        int encryptionFlag;
        int sampleCount;
        int sampleOffset;
        int sampleSize;
        int Padding;
    } Header = {};

    fs.read((char *)&Header, sizeof(M30Header));

    for (int i = 0; i < Header.sampleSize; i++) {
        struct M30SampleHeader
        {
            char  sampleName[32];
            int   sampleSize;
            short codecCode;
            short unkFixed;
            int   unkMusicFlag;
            short ValueRef;
            short unkFixed2;
            int   pcmSamples;
        } SampleHeader = {};

        fs.read((char *)&SampleHeader, sizeof(M30SampleHeader));
        if (SampleHeader.sampleSize == 0) {
            continue;
        }

        uint8_t *buffer = new uint8_t[SampleHeader.sampleSize];
        fs.read((char *)buffer, SampleHeader.sampleSize);

        switch (Header.encryptionFlag) {
            case 0:
                break;
            case 16:
            {
                M30Xor((char *)buffer, SampleHeader.sampleSize, MASK_NAMI);
                break;
            }
            case 32:
            {
                M30Xor((char *)buffer, SampleHeader.sampleSize, MASK_0412);
                break;
            }
        }

        // OGG Sample
        if (SampleHeader.codecCode == 0) {
            SampleHeader.ValueRef += 1000;
        }

        O2Sample sample = {};
        sample.RefValue = SampleHeader.ValueRef;
        sample.AudioData.insert(sample.AudioData.end(), buffer, buffer + SampleHeader.sampleSize);

        Samples.push_back(sample);
        delete[] buffer;
    }
}

void OJM::LoadOJMData(std::fstream &fs, bool encrypted)
{
    struct OJMHeader
    {
        short wavSizes;
        short oggSizes;
        int   wavOffset;
        int   oggOffset;
        int   fileSize;
    } Header = {};

    fs.read((char *)&Header, sizeof(OJMHeader));
    fs.seekg(Header.wavOffset, std::ios::beg);

    accKeyByte = 0xFF;
    accCounter = 0;
    int ValueRef = 0;

    for (int i = 0; i < Header.wavSizes; i++) {
        struct OJMWavSampleHeader
        {
            char  sampleName[32];
            short audioFormat;
            short channels;
            int   sampleRate;
            int   byteRate;
            short blockAlign;
            short bitsPerSample;
            int   unk1;
            int   chunkSize;
        } SampleHeader = {};

        fs.read((char *)&SampleHeader, sizeof(OJMWavSampleHeader));

        if (SampleHeader.chunkSize == 0) {
            ValueRef++;
            continue;
        }

        uint8_t *buffer = new uint8_t[SampleHeader.chunkSize];
        fs.read((char *)buffer, SampleHeader.chunkSize);

        if (encrypted) {
            auto data = WeirdRearrange((char *)buffer, SampleHeader.chunkSize);
            data = XorDecrypt(data);

            SampleHeader.chunkSize = (int)data.size();
            delete[] buffer;

            buffer = new uint8_t[SampleHeader.chunkSize];
            memcpy(buffer, data.data(), SampleHeader.chunkSize);
        }

        // flipArray(buffer, SampleHeader.chunkSize);
        std::stringstream ss;

        const char *RIFF = "RIFF";
        const char *WAVE = "WAVE";
        const char *fmt = "fmt ";

        ss.write(RIFF, 4);
        ss.write((char *)&SampleHeader.chunkSize + 36, 4);
        ss.write(WAVE, 4);
        ss.write(fmt, 4);

        int subchunk1Size = 0x10;
        ss.write((char *)&subchunk1Size, 4);
        ss.write((char *)&SampleHeader.audioFormat, 2);
        ss.write((char *)&SampleHeader.channels, 2);
        ss.write((char *)&SampleHeader.sampleRate, 4);
        ss.write((char *)&SampleHeader.byteRate, 4);
        ss.write((char *)&SampleHeader.blockAlign, 2);
        ss.write((char *)&SampleHeader.bitsPerSample, 2);

        const char *data = "data";
        ss.write(data, 4);
        ss.write((char *)&SampleHeader.chunkSize, 4);
        ss.write((char *)buffer, SampleHeader.chunkSize);

        O2Sample sample = {};
        sample.RefValue = ValueRef++;
        sample.AudioData.resize(ss.str().size());
        memcpy(sample.AudioData.data(), ss.str().data(), ss.str().size());

        auto utf8_name = CodepageToUtf8(SampleHeader.sampleName, sizeof(SampleHeader.sampleName), "euc-kr");
        memcpy(sample.FileName, utf8_name.c_str(), sizeof(sample.FileName));

        delete[] buffer;
        ss.str("");
        ss.clear();

        Samples.push_back(sample);
    }

    fs.seekg(Header.oggOffset, std::ios::beg);
    ValueRef = 1000;

    for (int i = 0; i < Header.oggSizes; i++) {
        struct OJMOggHeader
        {
            char sampleName[32];
            int  sampleSize;
        } SampleHeader = {};

        fs.read((char *)&SampleHeader, sizeof(OJMOggHeader));

        if (SampleHeader.sampleSize == 0) {
            ValueRef++;
            continue;
        }

        uint8_t *buffer = new uint8_t[SampleHeader.sampleSize];
        fs.read((char *)buffer, SampleHeader.sampleSize);

        O2Sample sample = {};
        sample.RefValue = ValueRef++;
        sample.AudioData.resize(SampleHeader.sampleSize);
        memcpy(sample.AudioData.data(), buffer, SampleHeader.sampleSize);

        auto utf8_name = CodepageToUtf8(SampleHeader.sampleName, sizeof(SampleHeader.sampleName), "euc-kr");
        memcpy(sample.FileName, utf8_name.c_str(), sizeof(sample.FileName));

        Samples.push_back(sample);
        delete[] buffer;
    }
}
