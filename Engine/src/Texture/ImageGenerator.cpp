#include "Texture/ImageGenerator.h"
#include "Misc/Lodepng.h"

#include <iostream>
#include <stdint.h>

Color interpolate(const Color &start, const Color &end, float t)
{
    return {
        static_cast<unsigned char>(start.r + t * (end.r - start.r)),
        static_cast<unsigned char>(start.g + t * (end.g - start.g)),
        static_cast<unsigned char>(start.b + t * (end.b - start.b)),
        static_cast<unsigned char>(start.a + t * (end.a - start.a))
    };
}

std::vector<uint8_t> ImageGenerator::GenerateGradientImage(int width, int height, std::vector<Segment> &segments)
{
    std::vector<uint8_t> results;

    // Compute the startPixel and endPixel for each segment based on the height
    for (Segment &seg : segments) {
        seg.startPixel = static_cast<int>(seg.startRatio * height);
        seg.endPixel = static_cast<int>(seg.endRatio * height);
    }

    std::vector<unsigned char> image(width * height * 4);

    for (int y = 0; y < height; ++y) {
        Color pixelColor = { 0, 0, 0, 255 };
        for (const Segment &seg : segments) {
            if (y >= seg.startPixel && y <= seg.endPixel) {
                float t = static_cast<float>(y - seg.startPixel) / (seg.endPixel - seg.startPixel);
                pixelColor = interpolate(seg.startColor, seg.endColor, t);
                break;
            }
        }

        for (int x = 0; x < width; ++x) {
            int index = 4 * width * y + 4 * x;
            image[index] = pixelColor.r;
            image[index + 1] = pixelColor.g;
            image[index + 2] = pixelColor.b;
            image[index + 3] = pixelColor.a;
        }
    }

    std::vector<uint8_t> result;
    int                  error = lodepng::encode(result, image, width, height, LCT_RGBA, 8);
    if (error != 0) {
        throw std::runtime_error("ImageGenerator::Fade: Failed to encode image");
    }

    return result;
}

std::vector<uint8_t> ImageGenerator::GenerateImage(int width, int height, Color color)
{
    std::vector<uint8_t> tempData(width * height * 4);

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            size_t index = (size_t)4 * width * y + (size_t)4 * x;
            tempData[index] = color.r;
            tempData[(size_t)(index + 1)] = color.g;
            tempData[(size_t)(index + 2)] = color.b;
            tempData[(size_t)(index + 3)] = color.a;
        }
    }

    std::vector<uint8_t> result;
    int                  error = lodepng::encode(result, tempData, width, height, LCT_RGBA, 8);
    if (error != 0) {
        throw std::runtime_error("ImageGenerator::GenerateImage: Failed to encode image");
    }

    return result;
}