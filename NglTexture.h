#pragma once

#include "nglgl.h"

class NglTexture {
public:
    NglTexture();
    NglTexture(const NglTexture&) = delete;
    NglTexture& operator=(const NglTexture&) = delete;
    NglTexture(NglTexture&&) = delete;
    NglTexture& operator=(NglTexture&&) = delete;
    ~NglTexture();

    operator GLuint() const;

    void load(const char* path) const;
    void load(const void* data, uint32_t length, const char* label) const;

    void bind(GLuint unit) const;

private:
    void load(void* pixels, int width, int height, const char* label) const;

    const GLuint mName;
};
