#pragma once

#include <random>
#include "sokol_log.h"

#define LOG(msg) slog_func("fountain", 3, 0, msg, __LINE__, __FILE__, nullptr)

inline float random(float min, float max)
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> distr(min, max);
    return static_cast<float>(distr(gen));
}

// File path utility function
inline const char* fileutil_get_path(const char* filename, char* buf, size_t buf_size)
{
    snprintf(buf, buf_size, "%s", filename);
    return buf;
}