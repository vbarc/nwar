#include "NglDisplacementMap.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "nglassert.h"
#include "ngllog.h"

NglDisplacementMap::NglDisplacementMap(const char* path) {
    mData = stbi_load(path, &mWidth, &mDepth, nullptr, STBI_grey);
    NGL_ASSERT(mData);
    NGL_ASSERT(mWidth > 0);
    NGL_ASSERT(mDepth > 0);
    NGL_LOGI("%s loaded, width: %d, depth: %d", path, mWidth, mDepth);
    for (int r = 0; r < mDepth; r++) {
        for (int c = 0; c < mWidth; c++) {
            NGL_LOGI("(%02d, %02d): %d", r, c, mData[r * mWidth + c]);
        }
    }
}

NglDisplacementMap::~NglDisplacementMap() {
    stbi_image_free(mData);
}

int NglDisplacementMap::width() const {
    return mWidth;
}

int NglDisplacementMap::depth() const {
    return mDepth;
}

float NglDisplacementMap::lookup(int x, int z) const {
    NGL_ASSERT(x >= 0);
    NGL_ASSERT(x < mWidth);
    NGL_ASSERT(z >= 0);
    NGL_ASSERT(z < mDepth);
    return mData[z * mWidth + x] / 255.0f;
}
