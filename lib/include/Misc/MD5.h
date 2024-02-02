#ifndef MD5_H
#define MD5_H

#include <filesystem>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct
{
    uint64_t size;       // Size of input in bytes
    uint32_t buffer[4];  // Current accumulation of hash
    uint8_t  input[64];  // Input to be used in the next step
    uint8_t  digest[16]; // Result of algorithm
} MD5Context;

void md5Init(MD5Context *ctx);
void md5Update(MD5Context *ctx, uint8_t *input, size_t input_len);
void md5Finalize(MD5Context *ctx);
void md5Step(uint32_t *buffer, uint32_t *input);

void md5Buffer(char *input, size_t input_len, uint8_t *result);
void md5String(char *input, uint8_t *result);
void md5File(FILE *file, uint8_t *result);

namespace Misc {
    namespace MD5 {
        std::string HashString(std::string input);
        std::string HashBuffer(char *input, size_t input_len);
        std::string HashFile(std::filesystem::path path);
    } // namespace MD5
} // namespace Misc

#endif