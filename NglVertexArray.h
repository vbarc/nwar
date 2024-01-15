#pragma once

#include "nglgl.h"

class NglVertexArray {
public:
    NglVertexArray();
    NglVertexArray(const NglVertexArray&) = delete;
    NglVertexArray& operator=(const NglVertexArray&) = delete;
    NglVertexArray(NglVertexArray&&) = delete;
    NglVertexArray& operator=(NglVertexArray&&) = delete;
    ~NglVertexArray();

    operator GLuint() const;

private:
    const GLuint mName;
};
