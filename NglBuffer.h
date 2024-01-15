#pragma once

#include "nglgl.h"

class NglBuffer {
public:
    NglBuffer();
    NglBuffer(const NglBuffer&) = delete;
    NglBuffer& operator=(const NglBuffer&) = delete;
    NglBuffer(NglBuffer&&) = delete;
    NglBuffer& operator=(NglBuffer&&) = delete;
    ~NglBuffer();

    operator GLuint() const;

private:
    const GLuint mName;
};
