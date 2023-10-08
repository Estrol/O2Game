/**
 * This code is taken from Imgui_draw.cpp
 * The code author is Omar Cornut
*/

#include "UIMath.h"
#include <Imgui/imgui_internal.h>

constexpr float PI = 3.14159265358979323846f;
constexpr int kArcFastMax = 48;
constexpr int kCircleSegmentCount = 64;

namespace {
    ImVec2 ArcFastVtx[kArcFastMax];
    uint8_t CircleSegmentCounts[kCircleSegmentCount];
    float ArcFastRadiusCutoff = 0;
    float CircleSegmentMaxError = 0;

    bool Initialized = false;
}

void Initialize(float max_error) {
    memset(CircleSegmentCounts, 0, sizeof(CircleSegmentCounts));
    memset(ArcFastVtx, 0, sizeof(ArcFastVtx));

    for (int i = 0; i < kArcFastMax; i++) {
        const float a = ((float)i * 2 * PI) / (float)kArcFastMax;
        ArcFastVtx[i] = ImVec2(cosf(a), sinf(a));
    }

    if (max_error == ArcFastRadiusCutoff) {
        return;
    }

    CircleSegmentMaxError = max_error;

    for (int i = 0; i < kCircleSegmentCount; i++) {
        const float radius = (float)i;
        CircleSegmentCounts[i] = (i > 0) ? IM_DRAWLIST_CIRCLE_AUTO_SEGMENT_CALC(radius, CircleSegmentMaxError) : IM_DRAWLIST_ARCFAST_SAMPLE_MAX;
    } 

    ArcFastRadiusCutoff = IM_DRAWLIST_CIRCLE_AUTO_SEGMENT_CALC_R(IM_DRAWLIST_ARCFAST_SAMPLE_MAX, CircleSegmentMaxError);
}

float _CalcCircleAutoSegmentCount(float radius) {
    // Automatic segment count
    const int radius_idx = (int)(radius + 0.999999f); // ceil to never reduce accuracy
    if (radius_idx >= 0 && radius_idx < kCircleSegmentCount)
        return CircleSegmentCounts[radius_idx]; // Use cached value
    else
        return IM_DRAWLIST_CIRCLE_AUTO_SEGMENT_CALC(radius, CircleSegmentMaxError);
}

void _PathArcToN(std::vector<ImVec2>& _Path, const ImVec2& center, float radius, float a_min, float a_max, int num_segments) {
    if (radius < 0.5f)
    {
        _Path.push_back(center);
        return;
    }

    // Note that we are adding a point at both a_min and a_max.
    // If you are trying to draw a full closed circle you don't want the overlapping points!
    _Path.reserve(_Path.size() + (num_segments + 1));
    for (int i = 0; i <= num_segments; i++)
    {
        const float a = a_min + ((float)i / (float)num_segments) * (a_max - a_min);
        _Path.push_back(ImVec2(center.x + ImCos(a) * radius, center.y + ImSin(a) * radius));
    }
}

void _PathArcToFastEx(std::vector<ImVec2>& _Path, const ImVec2& center, float radius, int a_min_sample, int a_max_sample, int a_step) {
    if (radius < 0.5f)
    {
        _Path.push_back(center);
        return;
    }

    // Calculate arc auto segment step size
    if (a_step <= 0)
        a_step = IM_DRAWLIST_ARCFAST_SAMPLE_MAX / _CalcCircleAutoSegmentCount(radius);

    // Make sure we never do steps larger than one quarter of the circle
    a_step = ImClamp(a_step, 1, IM_DRAWLIST_ARCFAST_TABLE_SIZE / 4);

    const int sample_range = ImAbs(a_max_sample - a_min_sample);
    const int a_next_step = a_step;

    int samples = sample_range + 1;
    bool extra_max_sample = false;
    if (a_step > 1)
    {
        samples            = sample_range / a_step + 1;
        const int overstep = sample_range % a_step;

        if (overstep > 0)
        {
            extra_max_sample = true;
            samples++;

            // When we have overstep to avoid awkwardly looking one long line and one tiny one at the end,
            // distribute first step range evenly between them by reducing first step size.
            if (sample_range > 0)
                a_step -= (a_step - overstep) / 2;
        }
    }

    _Path.resize(_Path.size() + samples);
    ImVec2* out_ptr = _Path.data() + (_Path.size() - samples);

    int sample_index = a_min_sample;
    if (sample_index < 0 || sample_index >= IM_DRAWLIST_ARCFAST_SAMPLE_MAX)
    {
        sample_index = sample_index % IM_DRAWLIST_ARCFAST_SAMPLE_MAX;
        if (sample_index < 0)
            sample_index += IM_DRAWLIST_ARCFAST_SAMPLE_MAX;
    }

    if (a_max_sample >= a_min_sample)
    {
        for (int a = a_min_sample; a <= a_max_sample; a += a_step, sample_index += a_step, a_step = a_next_step)
        {
            // a_step is clamped to IM_DRAWLIST_ARCFAST_SAMPLE_MAX, so we have guaranteed that it will not wrap over range twice or more
            if (sample_index >= IM_DRAWLIST_ARCFAST_SAMPLE_MAX)
                sample_index -= IM_DRAWLIST_ARCFAST_SAMPLE_MAX;

            const ImVec2 s = ArcFastVtx[sample_index];
            out_ptr->x = center.x + s.x * radius;
            out_ptr->y = center.y + s.y * radius;
            out_ptr++;
        }
    }
    else
    {
        for (int a = a_min_sample; a >= a_max_sample; a -= a_step, sample_index -= a_step, a_step = a_next_step)
        {
            // a_step is clamped to IM_DRAWLIST_ARCFAST_SAMPLE_MAX, so we have guaranteed that it will not wrap over range twice or more
            if (sample_index < 0)
                sample_index += IM_DRAWLIST_ARCFAST_SAMPLE_MAX;

            const ImVec2 s = ArcFastVtx[sample_index];
            out_ptr->x = center.x + s.x * radius;
            out_ptr->y = center.y + s.y * radius;
            out_ptr++;
        }
    }

    if (extra_max_sample)
    {
        int normalized_max_sample = a_max_sample % IM_DRAWLIST_ARCFAST_SAMPLE_MAX;
        if (normalized_max_sample < 0)
            normalized_max_sample += IM_DRAWLIST_ARCFAST_SAMPLE_MAX;

        const ImVec2 s = ArcFastVtx[normalized_max_sample];
        out_ptr->x = center.x + s.x * radius;
        out_ptr->y = center.y + s.y * radius;
        out_ptr++;
    }
}

void PathArc(std::vector<ImVec2>& _Path, const ImVec2& center, float radius, float a_min, float a_max, int num_segments) {
    if (radius < 0.5f)
    {
        _Path.push_back(center);
        return;
    }

    if (num_segments > 0)
    {
        _PathArcToN(_Path, center, radius, a_min, a_max, num_segments);
        return;
    }

    if (radius <= ArcFastRadiusCutoff) {
        const bool a_is_reverse = a_max < a_min;

        const float a_min_sample_f = IM_DRAWLIST_ARCFAST_SAMPLE_MAX * a_min / (IM_PI * 2.0f);
        const float a_max_sample_f = IM_DRAWLIST_ARCFAST_SAMPLE_MAX * a_max / (IM_PI * 2.0f);

        const int a_min_sample = a_is_reverse ? (int)ImFloor(a_min_sample_f) : (int)ImCeil(a_min_sample_f);
        const int a_max_sample = a_is_reverse ? (int)ImCeil(a_max_sample_f) : (int)ImFloor(a_max_sample_f);
        const int a_mid_samples = a_is_reverse ? ImMax(a_min_sample - a_max_sample, 0) : ImMax(a_max_sample - a_min_sample, 0);

        const float a_min_segment_angle = a_min_sample * IM_PI * 2.0f / IM_DRAWLIST_ARCFAST_SAMPLE_MAX;
        const float a_max_segment_angle = a_max_sample * IM_PI * 2.0f / IM_DRAWLIST_ARCFAST_SAMPLE_MAX;
        const bool a_emit_start = ImAbs(a_min_segment_angle - a_min) >= 1e-5f;
        const bool a_emit_end = ImAbs(a_max - a_max_segment_angle) >= 1e-5f;

        _Path.reserve(_Path.size() + (a_mid_samples + 1 + (a_emit_start ? 1 : 0) + (a_emit_end ? 1 : 0)));

        if (a_emit_start)
            _Path.push_back(ImVec2(center.x + ImCos(a_min) * radius, center.y + ImSin(a_min) * radius));
        if (a_mid_samples > 0)
            _PathArcToFastEx(_Path, center, radius, a_min_sample, a_max_sample, 0);
        if (a_emit_end)
            _Path.push_back(ImVec2(center.x + ImCos(a_max) * radius, center.y + ImSin(a_max) * radius));
    }
    else {
        const float arc_length = ImAbs(a_max - a_min);
        const int circle_segment_count = _CalcCircleAutoSegmentCount(radius);
        const int arc_segment_count = ImMax((int)ImCeil(circle_segment_count * arc_length / (IM_PI * 2.0f)), (int)(2.0f * IM_PI / arc_length));
        _PathArcToN(_Path, center, radius, a_min, a_max, arc_segment_count);
    }
}

void PathArcToFast(std::vector<ImVec2>& _Path, const ImVec2& center, float radius, int a_min_of_12, int a_max_of_12) {
    if (radius < 0.5f) {
        _Path.push_back(center);
        return;
    }

    _PathArcToFastEx(_Path, center, radius, a_min_of_12 * IM_DRAWLIST_ARCFAST_SAMPLE_MAX / 12, a_max_of_12 * IM_DRAWLIST_ARCFAST_SAMPLE_MAX / 12, 0);
}

void PathRect(std::vector<ImVec2>& _Path, const ImVec2& a, const ImVec2& b, float rounding, ImDrawFlags flags) {
    if (rounding >= 0.5f) {
        rounding = ImMin(rounding, ImFabs(b.x - a.x) * (((flags & ImDrawFlags_RoundCornersTop) == ImDrawFlags_RoundCornersTop) || ((flags & ImDrawFlags_RoundCornersBottom) == ImDrawFlags_RoundCornersBottom) ? 0.5f : 1.0f) - 1.0f);
        rounding = ImMin(rounding, ImFabs(b.y - a.y) * (((flags & ImDrawFlags_RoundCornersLeft) == ImDrawFlags_RoundCornersLeft) || ((flags & ImDrawFlags_RoundCornersRight) == ImDrawFlags_RoundCornersRight) ? 0.5f : 1.0f) - 1.0f);
    }

    const float rounding_tl = (flags & ImDrawFlags_RoundCornersTopLeft)     ? rounding : 0.0f;
    const float rounding_tr = (flags & ImDrawFlags_RoundCornersTopRight)    ? rounding : 0.0f;
    const float rounding_br = (flags & ImDrawFlags_RoundCornersBottomRight) ? rounding : 0.0f;
    const float rounding_bl = (flags & ImDrawFlags_RoundCornersBottomLeft)  ? rounding : 0.0f;
    PathArcToFast(_Path, ImVec2(a.x + rounding_tl, a.y + rounding_tl), rounding_tl, 6, 9);
    PathArcToFast(_Path, ImVec2(b.x - rounding_tr, a.y + rounding_tr), rounding_tr, 9, 12);
    PathArcToFast(_Path, ImVec2(b.x - rounding_br, b.y - rounding_br), rounding_br, 0, 3);
    PathArcToFast(_Path, ImVec2(a.x + rounding_bl, b.y - rounding_bl), rounding_bl, 3, 6);
}

std::vector<ImDrawVert> UIMath::GenerateQuadVertices(ImVec4 rect, ImVec4 color, float radius) {
    if (!Initialized) {
        Initialize(0.30f);
    }

    ImU32 col = ImGui::ColorConvertFloat4ToU32(color);
    if ((col & IM_COL32_A_MASK) == 0)
        return {};

    if (radius < 0.0f) {
        std::vector<ImDrawVert> result = {};
        auto x1 = rect.x;
        auto y1 = rect.y;
        auto x2 = rect.w;
        auto y2 = rect.z;

        for (int i = 0; i < 6; i++) {
            ImDrawVert& vertex = result[i];
            switch (i) {
                case 0:
                    vertex.pos = ImVec2(x1, y1);
                    break;
                case 1:
                    vertex.pos = ImVec2(x2, y1);
                    break;
                case 2:
                    vertex.pos = ImVec2(x2, y2);
                    break;
                case 3:
                    vertex.pos = ImVec2(x1, y1);
                    break;
                case 4:
                    vertex.pos = ImVec2(x2, y2);
                    break;
                case 5:
                    vertex.pos = ImVec2(x1, y2);
                    break;
            }

            vertex.col = col;
        }

        return result;
    } else {
        std::vector<ImVec2> coords = {};
        
    }
}
