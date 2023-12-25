#pragma once

#include "ngllog.h"

#define NGL_STR(x) #x

#define NGL_ASSERT(condition)                                                                                     \
    do {                                                                                                          \
        if (!(condition)) {                                                                                       \
            NGL_LOGE("Assertion failed at %s:%d [%s]: %s", __FILE__, __LINE__, __FUNCTION__, NGL_STR(condition)); \
            abort();                                                                                              \
        }                                                                                                         \
    } while (false)
