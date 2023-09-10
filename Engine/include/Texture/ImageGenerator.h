#pragma once
#include <string>
#include <vector>

struct Color {
    unsigned char r, g, b, a;
};

struct Segment {
    float startRatio, endRatio;  // Using ratios instead of absolute heights
    Color startColor, endColor;
    int startPixel, endPixel;    // Computed pixel positions based on height
};

namespace ImageGenerator {
    std::vector<uint8_t> GenerateGradientImage(int width, int height, std::vector<Segment>& segments);
}