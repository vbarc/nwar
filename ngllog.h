#pragma once

#include <cstdio>

#define NGL_LOG(level, format, ...) printf(level ": " format "\n", __VA_ARGS__)
#define NGL_LOGE(format, ...) NGL_LOG("ERROR", format, __VA_ARGS__)
#define NGL_LOGI(format, ...) NGL_LOG("INFO", format, __VA_ARGS__)
