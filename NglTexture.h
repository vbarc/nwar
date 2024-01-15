#pragma once

#include "nglgl.h"

class NglTexture {
public:
    NglTexture(const char* filename);
    NglTexture(const void* data, uint32_t length, const char* label);
    NglTexture(const NglTexture&) = delete;
    NglTexture& operator=(const NglTexture&) = delete;
    NglTexture(NglTexture&&) noexcept;
    NglTexture& operator=(NglTexture&&) noexcept;
    ~NglTexture();

    void bind(GLuint unit);

private:
    static GLuint load(const char* filename);
    static GLuint load(const void* data, uint32_t length, const char* label);
    static GLuint load(void* pixels, int width, int height, const char* label);

    const GLuint mName;
};
