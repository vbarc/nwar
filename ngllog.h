#pragma once

#define NGL_LOG(level, ...) printf(level ": " __VA_ARGS__)
#define NGL_LOGE(...) NGL_LOG("ERROR", __VA_ARGS__)
