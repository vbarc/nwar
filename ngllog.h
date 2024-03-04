#pragma once

#include <cstdio>

#define NGL_LOG(level, format, ...) printf("%s: " format "\n", level, __VA_ARGS__)
#define NGL_LOGE(format, ...) NGL_LOG("ERROR", format, __VA_ARGS__)
#define NGL_LOGI(format, ...) NGL_LOG("INFO", format, __VA_ARGS__)

#define NGL_ABORT(format, ...)                                                       \
    do {                                                                             \
        NGL_LOGE("Fatal error at %s(%d): " format, __FILE__, __LINE__, __VA_ARGS__); \
        abort();                                                                     \
    } while (false)
