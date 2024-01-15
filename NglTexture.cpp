#include "NglTexture.h"

#include <stb_image.h>

#include "nglassert.h"
#include "nglerr.h"
#include "ngllog.h"

NglTexture::NglTexture(const char* filename) : mName(load(filename)) {}

NglTexture::NglTexture(const void* data, uint32_t length, const char* label) : mName(load(data, length, label)) {}

NglTexture::~NglTexture() {
    glDeleteTextures(1, &mName);
    NGL_CHECK_ERRORS;
}

void NglTexture::bind(GLuint unit) {
    glBindTextures(unit, 1, &mName);
    NGL_CHECK_ERRORS;
}

GLuint NglTexture::load(const char* filename) {
    int width;
    int height;
    void* pixels = stbi_load(filename, &width, &height, nullptr, STBI_rgb);
    return load(pixels, width, height, filename);
}

GLuint NglTexture::load(const void* data, uint32_t length, const char* label) {
    int width;
    int height;
    void* pixels =
            stbi_load_from_memory(reinterpret_cast<const stbi_uc*>(data), length, &width, &height, nullptr, STBI_rgb);
    return load(pixels, width, height, label);
}

GLuint NglTexture::load(void* pixels, int width, int height, const char* label) {
    NGL_ASSERT(pixels);

    GLuint name;
    glCreateTextures(GL_TEXTURE_2D, 1, &name);
    NGL_CHECK_ERRORS;
    glTextureParameteri(name, GL_TEXTURE_MAX_LEVEL, 0);
    NGL_CHECK_ERRORS;
    glTextureParameteri(name, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    NGL_CHECK_ERRORS;
    glTextureParameteri(name, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    NGL_CHECK_ERRORS;
    glTextureStorage2D(name, 1, GL_RGB8, width, height);
    NGL_CHECK_ERRORS;
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    NGL_CHECK_ERRORS;
    glTextureSubImage2D(name, 0, 0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, pixels);
    NGL_CHECK_ERRORS;

    stbi_image_free(pixels);

    NGL_LOGI("Texture %s loaded, width: %d, height: %d", label, width, height);
    return name;
}
