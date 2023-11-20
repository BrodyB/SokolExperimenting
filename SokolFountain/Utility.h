#pragma once

#include <random>
#include "sokol_log.h"

#define LOG(msg) slog_func("fountain", 3, 0, msg, __LINE__, __FILE__, nullptr)

static std::random_device rd;
static std::mt19937 gen(rd());

inline float random(float min, float max)
{
    std::uniform_real_distribution<> distr(min, max);
    return static_cast<float>(distr(gen));
}

// File path utility function
inline const char* fileutil_get_path(const char* filename, char* buf, size_t buf_size)
{
    snprintf(buf, buf_size, "%s", filename);
    return buf;
}

inline float clamp(float value, float min, float max)
{
    if (value < min) return min;
    if (value > max) return max;
    return value;
}

inline float lerp(float a, float b, float t)
{
    t = clamp(t, 0.0f, 1.0f);
    return a * (1.0f - t) + (b * t);
}