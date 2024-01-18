#include "NglTexture.h"

#include <stb_image.h>

#include "nglassert.h"
#include "nglerr.h"
#include "ngllog.h"

static GLuint create() {
    GLuint name;
    glCreateTextures(GL_TEXTURE_2D, 1, &name);
    NGL_CHECK_ERRORS;
    NGL_ASSERT(name);
    return name;
}

NglTexture::NglTexture() : mName(create()) {}

NglTexture::~NglTexture() {
    glDeleteTextures(1, &mName);
    NGL_CHECK_ERRORS;
}

NglTexture::operator GLuint() const {
    return mName;
}

void NglTexture::load(const char* path) const {
    int width;
    int height;
    void* pixels = stbi_load(path, &width, &height, nullptr, STBI_rgb);
    load(pixels, width, height, path);
}

void NglTexture::load(const void* data, uint32_t length, const char* label) const {
    int width;
    int height;
    void* pixels =
            stbi_load_from_memory(reinterpret_cast<const stbi_uc*>(data), length, &width, &height, nullptr, STBI_rgb);
    load(pixels, width, height, label);
}

void NglTexture::load(void* pixels, int width, int height, const char* label) const {
    NGL_ASSERT(pixels);

    glTextureParameteri(mName, GL_TEXTURE_MAX_LEVEL, 0);
    NGL_CHECK_ERRORS;
    glTextureParameteri(mName, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    NGL_CHECK_ERRORS;
    glTextureParameteri(mName, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    NGL_CHECK_ERRORS;
    glTextureStorage2D(mName, 1, GL_RGB8, width, height);
    NGL_CHECK_ERRORS;
    glTextureSubImage2D(mName, 0, 0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, pixels);
    NGL_CHECK_ERRORS;

    stbi_image_free(pixels);

    NGL_LOGI("Texture %s loaded, width: %d, height: %d", label, width, height);
}

void NglTexture::bind(GLuint unit) const {
    glBindTextureUnit(unit, mName);
    NGL_CHECK_ERRORS;
}
